// Copyright (c) 2022-2025 Manuel Schneider

#include "albert.h"
#include "app.h"
#include "frontend.h"
#include "messagebox.h"
#include "pluginswidget.h"
#include "querywidget.h"
#include "settingswindow.h"
#include "systemutil.h"
#include "telemetry.h"
#include <QDialog>
#include <QHotkey>
#include <QKeyEvent>
using namespace albert::util;
using namespace albert;
using namespace std;

const auto privacy_notice_url = "https://albertlauncher.github.io/privacy/";


class QHotKeyDialog : public QDialog
{
public:

    QHotKeyDialog(QWidget *parent) : QDialog(parent)
    {
        setWindowTitle(SettingsWindow::tr("Set hotkey"));
        setLayout(new QVBoxLayout);
        layout()->addWidget(&label);
        label.setText(SettingsWindow::tr("Press a key combination"));
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

            if (auto hk = make_unique<QHotkey>(keyEvent->keyCombination());
                hk->setRegistered(true))
            {
                label.setText(hk->shortcut().toString(QKeySequence::NativeText));
                hotkey = ::move(hk);
                accept();
            }
        }
        return QDialog::event(event);
    }

    QLabel label;
    std::unique_ptr<QHotkey> hotkey;
};


SettingsWindow::SettingsWindow(App &a):
    app(a),
    ui(),
    small_text_fmt(R"(<span style="font-size:9pt; color:#808080;">%1</span>)")
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    init_tab_general_hotkey();
    init_tab_general_trayIcon();
    init_tab_general_frontends();
    init_tab_general_path();
    init_tab_general_telemetry();
    init_tab_general_about();

    ui.tabs->insertTab(ui.tabs->count(), app.frontend()->createFrontendConfigWidget(), tr("&Window"));
    ui.tabs->insertTab(ui.tabs->count(), plugin_widget = new PluginsWidget(app.pluginRegistry()), tr("&Plugins"));
    ui.tabs->insertTab(ui.tabs->count(), new QueryWidget(app.queryEngine()), tr("&Query"));

    auto *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    auto geometry = screen->geometry();
    move(geometry.center().x() - frameSize().width()/2,
         geometry.top() + geometry.height() / 5);
}

SettingsWindow::~SettingsWindow() = default;

void SettingsWindow::init_tab_general_hotkey()
{
    if (QHotkey::isPlatformSupported())
    {
        if (const auto &hk = app.hotkey(); hk)
            ui.pushButton_hotkey->setText(hk->shortcut().toString(QKeySequence::NativeText));
        else
            ui.pushButton_hotkey->setText(tr("Not set"));

        connect(ui.pushButton_hotkey, &QPushButton::clicked, this, [this]{
            QHotKeyDialog dialog(this);
            if(dialog.exec() == QDialog::Accepted)
            {
                if (dialog.hotkey)
                {
                    app.setHotkey(::move(dialog.hotkey));
                    ui.pushButton_hotkey->
                        setText(app.hotkey()->shortcut().toString(QKeySequence::NativeText));
                }
                else
                {
                    app.setHotkey(nullptr);
                    ui.pushButton_hotkey->setText(tr("Not set"));
                }
            }
        });
    }
    else
    {
        ui.label_hotkey->setEnabled(false);
        ui.pushButton_hotkey->setText(tr("Not supported"));
        connect(ui.pushButton_hotkey, &QPushButton::clicked, this, []{
            openUrl("https://albertlauncher.github.io/gettingstarted/faq/#wayland");
        });
    }
}

void SettingsWindow::init_tab_general_trayIcon()
{
    ui.checkBox_showTray->setChecked(app.trayEnabled());
    connect(ui.checkBox_showTray, &QCheckBox::toggled,
            &app, &App::setTrayEnabled);
}

void SettingsWindow::init_tab_general_frontends()
{
    // Populate frontend checkbox
    for (const auto &name : app.availableFrontends())
    {
        ui.comboBox_frontend->addItem(name);
        if (name == app.currentFrontend())
            ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
    }
    connect(ui.comboBox_frontend, &QComboBox::currentIndexChanged, this,
            [this](int index){ app.setFrontend(index); });
}

void SettingsWindow::init_tab_general_path()
{
    const auto &additional = app.additionalPathEntries();
    const auto &original  = app.originalPathEntries();

    auto *le = ui.lineEdit_additional_path_entries;
    le->setPlaceholderText(original.join(":"));
    le->setText(additional.join(":"));
    le->setToolTip((additional + original).join(":"));

    connect(le, &QLineEdit::editingFinished,
            this, [this, le] {
                const auto new_add = le->text().split(":");

                if (new_add == app.additionalPathEntries())
                    return;

                app.setAdditionalPathEntries(new_add);
                le->setToolTip((new_add + app.originalPathEntries()).join(":"));

                if (question(tr("For the changes to take effect, Albert has to be restarted. "
                                "Do you want to restart Albert now?")))
                    restart();
            });
}

void SettingsWindow::init_tab_general_telemetry()
{
    ui.checkBox_telemetry->setToolTip(app.telemetry().buildReportString());
    ui.checkBox_telemetry->setIcon(style()->standardPixmap(QStyle::SP_MessageBoxQuestion));
    ui.checkBox_telemetry->setChecked(app.telemetry().enabled());

    connect(ui.checkBox_telemetry, &QCheckBox::toggled, this, [this](bool checked){
        app.telemetry().setEnabled(checked);
        ui.checkBox_telemetry->setToolTip(app.telemetry().buildReportString());
    });

    ui.label_telemetry->setText(QString("[%1](%2)")
                                    .arg(ui.label_telemetry->text(), privacy_notice_url));
}

void SettingsWindow::init_tab_general_about()
{
    ui.label_app->setText(QString("<b>%1 v%2</b>")
                          .arg(qApp->applicationDisplayName(),
                               qApp->applicationVersion()));

    QStringList links;
    links << QStringLiteral("[Telegram](https://telegram.me/albert_launcher_community)");
    links << QStringLiteral("[Discord](https://discord.com/invite/t8G2EkvRZh)");
    links << QStringLiteral("[News](https://albertlauncher.github.io/news/)");
    links << QStringLiteral("[GitHub](https://github.com/albertlauncher)");
    links << QStringLiteral("[Donate](https://albertlauncher.github.io/donation/)");
    ui.label_links->setText(links.join(" · "));

    QStringList credits;
    credits << ui.label_credits->text();
    credits << "QHotkey - Felix Barz  (BSD-3-Clause)";
    credits << "qtkeychain - Frank Osterfeld  (BSD-3-Clause)";
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
