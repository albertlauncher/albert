// Copyright (c) 2022-2026 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "frontendregistry.h"
#include "hotkey.h"
#include "hotkeymanager.h"
#include "localization.h"
#include "messagebox.h"
#include "pathmanager.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginswidget.h"
#include "querywidget.h"
#include "settingswindow.h"
#include "systemtrayicon.h"
#include "systemutil.h"
#include "telemetry.h"
#include <QDialog>
#include <QKeyEvent>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

const auto *privacy_notice_url = "https://albertlauncher.github.io/privacy/";
const char *restart_required_text
    = QT_TRANSLATE_NOOP("SettingsWindow",
                        "For the changes to take effect, Albert has to be restarted. "
                        "Do you want to restart Albert now?");

class HotKeyDialog : public QDialog
{
public:
    HotKeyDialog(QWidget *parent) : QDialog(parent)
    {
        setWindowTitle(SettingsWindow::tr("Set hotkey"));
        setLayout(new QVBoxLayout);
        layout()->addWidget(&label);
        label.setText(SettingsWindow::tr("Press a key combination\nBackspace to clear · Escape to cancel"));
        label.setAlignment(Qt::AlignCenter);
        setWindowModality(Qt::WindowModal);
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::KeyPress){
            auto *keyEvent = static_cast<QKeyEvent*>(event);

            if (Qt::Key_Shift <= keyEvent->key() && keyEvent->key() <= Qt::Key_ScrollLock)
                return false; // Filter mod keys

            if (keyEvent->modifiers() == Qt::NoModifier)
            {
                if (keyEvent->key() == Qt::Key_Escape)
                    reject();
                else if (keyEvent->key() == Qt::Key_Backspace)
                    accept();
                return true;
            }

            if (auto exp_hk = Hotkey::grab(keyEvent->keyCombination()))
            {
                hotkey = ::move(*exp_hk);
                label.setText(hotkey->nativeString());
                accept();
            }
        }
        return QDialog::event(event);
    }

    QLabel label;
    unique_ptr<Hotkey> hotkey;
};

SettingsWindow::SettingsWindow(FrontendRegistry &fronten_registry,
                               HotkeyManager &hotkey_manager,
                               Localization &localization,
                               PathManager &path_manager,
                               PluginRegistry &plugin_registry,
                               QueryEngine &query_engine,
                               SystemTrayIcon &tray_icon,
                               Telemetry &telemetry) :
    ui(),
    small_text_fmt(R"(<span style="font-size:9pt; color:#808080;">%1</span>)")
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    init_tab_general_hotkey(hotkey_manager);
    init_tab_general_trayIcon(tray_icon);
    init_tab_general_frontends(fronten_registry);
    init_tab_general_path(path_manager);
    init_tab_general_telemetry(telemetry);
    init_tab_general_localization(localization);
    init_tab_general_about();

    ui.tabs->insertTab(ui.tabs->count(),
                       fronten_registry.frontend().createFrontendConfigWidget(),
                       tr("&Window"));

    ui.tabs->insertTab(ui.tabs->count(),
                       plugin_widget = new PluginsWidget(plugin_registry),
                       tr("&Plugins"));

    ui.tabs->insertTab(ui.tabs->count(),
                       new QueryWidget(query_engine),
                       tr("&Query"));


    auto *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    auto geometry = screen->geometry();
    move(geometry.center().x() - frameSize().width()/2,
         geometry.top() + geometry.height() / 5);
}

SettingsWindow::~SettingsWindow() = default;

void SettingsWindow::init_tab_general_hotkey(HotkeyManager &hotkey_manager)
{
    if (Hotkey::isPlatformSupported())
    {
        if (const auto *hk = hotkey_manager.hotkey())
            ui.pushButton_hotkey->setText(hk->nativeString());
        else
            ui.pushButton_hotkey->setText(tr("Not set"));

        connect(ui.pushButton_hotkey, &QPushButton::clicked, this, [this, &hotkey_manager]{
            HotKeyDialog dialog(this);
            if(dialog.exec() == QDialog::Accepted)
            {
                if (dialog.hotkey)
                    ui.pushButton_hotkey->setText(dialog.hotkey->nativeString());
                else
                    ui.pushButton_hotkey->setText(tr("Not set"));
                hotkey_manager.setHotkey(::move(dialog.hotkey));
            }
        });
    }
    else
    {
        ui.label_hotkey->setEnabled(false);
        ui.pushButton_hotkey->setText(tr("Not supported"));
        connect(ui.pushButton_hotkey, &QPushButton::clicked, this, []{
            openUrl("https://albertlauncher.github.io/faq/#wayland");
        });
    }
}

void SettingsWindow::init_tab_general_trayIcon(SystemTrayIcon &tray_icon)
{
    ui.checkBox_showTray->setChecked(tray_icon.isEnabled());
    connect(ui.checkBox_showTray, &QCheckBox::toggled,
            this, [&tray_icon](bool enable) { tray_icon.setEnabled(enable); });
}

void SettingsWindow::init_tab_general_frontends(FrontendRegistry &frontend_registry)
{
    // Populate frontend checkbox
    for (const auto *plugin_loader : frontend_registry.availableFrontends())
    {
        const auto &id = plugin_loader->metadata().id;
        const auto &name = plugin_loader->metadata().name;

        ui.comboBox_frontend->addItem(name, id);
        if (id == frontend_registry.frontend().loader().metadata().id)
            ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
    }

    connect(ui.comboBox_frontend, &QComboBox::currentIndexChanged, this, [&](int index) {
        frontend_registry.setFrontend(index);

        if (frontend_registry.availableFrontends().at(index)->metadata().id
            != frontend_registry.frontend().loader().metadata().id)
            if (question(tr(restart_required_text)))
                App::restart();
    });
}

void SettingsWindow::init_tab_general_path(PathManager &path_manager)
{
    auto *le = ui.lineEdit_additional_path_entries;
    const auto &additional = path_manager.additionalPathEntries();
    const auto &original   = path_manager.originalPathEntries();

    le->setText(additional.join(":"));
    le->setToolTip((additional + original).join(":"));

    connect(le, &QLineEdit::editingFinished,
            this, [&path_manager, le] {
                const auto new_add = le->text().split(":");

                if (new_add == path_manager.additionalPathEntries())
                    return;

                path_manager.setAdditionalPathEntries(new_add);
                le->setToolTip((new_add + path_manager.originalPathEntries()).join(":"));

                if (question(tr(restart_required_text)))
                    App::restart();
            });
}

void SettingsWindow::init_tab_general_telemetry(Telemetry &telemetry)
{
    ui.checkBox_telemetry->setToolTip(telemetry.buildReportString());
    ui.checkBox_telemetry->setIcon(style()->standardPixmap(QStyle::SP_MessageBoxQuestion));
    ui.checkBox_telemetry->setChecked(telemetry.enabled());

    connect(ui.checkBox_telemetry, &QCheckBox::toggled, this, [this, &telemetry](bool checked){
        telemetry.setEnabled(checked);
        ui.checkBox_telemetry->setToolTip(telemetry.buildReportString());
    });

    ui.label_telemetry->setText(QString("[%1](%2)")
                                    .arg(ui.label_telemetry->text(), privacy_notice_url));
}

void SettingsWindow::init_tab_general_localization(Localization &localization)
{
    ui.checkBox_localization->setEnabled(localization.isAvailable());
    if (localization.isAvailable())
    {
        ui.checkBox_localization->setChecked(localization.isEnabled());
        connect(ui.checkBox_localization, &QCheckBox::toggled, this, [&localization](bool v){
            localization.setEnabled(v);
            if (question(tr(restart_required_text)))
                App::restart();
        });
    }
    else
        ui.checkBox_localization->setToolTip(tr("No translations available for your language."));
}

void SettingsWindow::init_tab_general_about()
{
    ui.label_app->setText(QString("<b>%1 v%2</b>")
                          .arg(qApp->applicationDisplayName(),
                               qApp->applicationVersion()));

    QStringList links;
    links << u"[Telegram](https://telegram.me/albert_launcher_community)"_s;
    links << u"[Discord](https://discord.com/invite/t8G2EkvRZh)"_s;
    links << u"[News](https://albertlauncher.github.io/news/)"_s;
    links << u"[GitHub](https://github.com/albertlauncher)"_s;
    links << u"[%1](https://albertlauncher.github.io/donation/)"_s.arg(tr("Donate"));
    ui.label_links->setText(links.join(" · "));

    QStringList credits;
    credits << ui.label_credits->text();
    credits << "QHotkey - Felix Barz  (BSD-3-Clause)";
    ui.label_credits->setText(small_text_fmt.arg(credits.join("<br>")));
}

void SettingsWindow::bringToFront(const QString &plugin)
{
    show();
    raise();
    activateWindow();
    if (!plugin.isNull()){
        plugin_widget->tryShowPluginSettings(plugin);
        ui.tabs->setCurrentWidget(plugin_widget);
    }
}

void SettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::NoModifier){

        if (event->key() == Qt::Key_Escape)
            close();

    } else if (event->modifiers() == Qt::ControlModifier){

        if(event->key() == Qt::Key_W)
            close();

        // Tab navi by number
        else if((int)Qt::Key_1 <= event->key() && event->key() < (int)Qt::Key_1 + ui.tabs->count())
            ui.tabs->setCurrentIndex((int)event->key() - (int)Qt::Key_1);
    }

    QWidget::keyPressEvent(event);
}
