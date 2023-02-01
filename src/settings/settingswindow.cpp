// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extensions/frontend.h"
#include "albert/logging.h"
#include "app.h"
#include "pluginwidget.h"
#include "settingswindow.h"
#include "trayicon.h"
#include "triggerwidget.h"
#include <QCloseEvent>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QKeySequenceEdit>
using namespace std;


class QHotKeyEdit : public QKeySequenceEdit
{
public:
    QHotKeyEdit(Hotkey &hk) : hotkey(hk)
    {
        if (!hotkey.isPlatformSupported()){
            setToolTip("This platform does not support hotkeys.");
            setEnabled(false);
        }
        else
            setKeySequence(hotkey.hotkey());
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::KeyPress){
            auto *keyEvent = static_cast<QKeyEvent*>(event);

            if (Qt::Key_Shift <= keyEvent->key() && keyEvent->key() <= Qt::Key_ScrollLock)
                return true; // Filter mod keys

            if (auto ckc = keyEvent->keyCombination(); hotkey.setHotkey(ckc))
                setKeySequence(ckc);

            return true;
        }
        return false;
    }

    Hotkey &hotkey;
};


SettingsWindow::SettingsWindow(App &app) : ui()
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui.tabs->setStyleSheet("QTabWidget::pane { border-radius: 0px; }");

    init_tab_general_hotkey(app.hotkey);
    init_tab_general_frontend(app.plugin_provider);
    init_tab_general_terminal(app.terminal_provider);
    init_tab_general_trayIcon(app.tray_icon);
    init_tab_general_autostart();
    init_tab_general_search(app.query_engine);

    ui.tabs->insertTab(ui.tabs->count()-1, new TriggerWidget(app.query_engine, app.extension_registry), "Triggers");
    ui.tabs->insertTab(ui.tabs->count()-1, new PluginWidget(app.plugin_registry), "Plugins");

    init_tab_about();

    auto geometry = QGuiApplication::screenAt(QCursor::pos())->geometry();
    move(geometry.center().x() - frameSize().width()/2,
         geometry.top() + geometry.height() / 5);
}

void SettingsWindow::init_tab_general_hotkey(Hotkey &Hotkey)
{
    ui.formLayout_general->insertRow(0, "Hotkey", new QHotKeyEdit(Hotkey));
}

void SettingsWindow::init_tab_general_frontend(NativePluginProvider &plugin_provider)
{
    for (const auto *loader : plugin_provider.frontendPlugins()){
        ui.comboBox_frontend->addItem(loader->metaData().name);
        if (loader->metaData().id == plugin_provider.frontend()->id())
            ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
    }

    connect(ui.comboBox_frontend, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [&plugin_provider](int index) { plugin_provider.setFrontend(index); });

    ui.groupBox_window->layout()->addWidget(plugin_provider.frontend()->createFrontendConfigWidget());
}

void SettingsWindow::init_tab_general_terminal(TerminalProvider &terminal_provider)
{
    for (const auto &terminal : terminal_provider.terminals()){
        ui.comboBox_term->addItem(terminal->name());
        if (terminal.get() == &terminal_provider.terminal())
            ui.comboBox_term->setCurrentIndex(ui.comboBox_term->count()-1);
    }

    connect(ui.comboBox_term, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [&terminal_provider](int index){ terminal_provider.setTerminal(index); });
}

void SettingsWindow::init_tab_general_trayIcon(TrayIcon &tray_icon)
{
    ui.checkBox_showTray->setChecked(tray_icon.isVisible());
    QObject::connect(ui.checkBox_showTray, &QCheckBox::toggled,
                     &tray_icon, &TrayIcon::setVisible);
}

void SettingsWindow::init_tab_general_autostart()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    QString desktopfile_path = QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                                      "albert.desktop",
                                                      QStandardPaths::LocateFile);
    if (!desktopfile_path.isNull()) {
        QString autostart_path = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).filePath("autostart/albert.desktop");
        ui.checkBox_autostart->setChecked(QFile::exists(autostart_path));
        connect(ui.checkBox_autostart, &QCheckBox::toggled,
                this, [=](bool toggled){
            if (toggled)
                QFile::link(desktopfile_path, autostart_path);
            else
                QFile::remove(autostart_path);
        });
    }
    else
        CRIT << "Deskop entry not found! Autostart option is nonfuctional";
#else
    ui.checkBox_autostart->setEnabled(false);
    WARN << "Autostart not implemented on this platform!";
#endif
}

void SettingsWindow::init_tab_general_search(QueryEngine &engine)
{
    ui.checkBox_fuzzy->setChecked(engine.fuzzy());
    QObject::connect(ui.checkBox_fuzzy, &QCheckBox::toggled,
                     [&engine](bool checked){ engine.setFuzzy(checked); });

    ui.lineEdit_separators->setText(engine.separators());
    QObject::connect(ui.lineEdit_separators, &QLineEdit::editingFinished,
                     [&](){ engine.setSeparators(ui.lineEdit_separators->text()); });

    ui.slider_decay->setValue((int)(engine.memoryDecay()*100));
    QObject::connect(ui.slider_decay, &QSlider::valueChanged,
                     [&](int val){ engine.setMemoryDecay((double)val/100.0); });

    ui.checkBox_prioritizePerfectMatch->setChecked(engine.prioritizePerfectMatch());
    QObject::connect(ui.checkBox_prioritizePerfectMatch, &QCheckBox::toggled,
                     [&](bool val){ engine.setPrioritizePerfectMatch(val); });
}

void SettingsWindow::init_tab_about()
{
    auto open_link = [](const QString &link){
        if( link == "aboutQt" ){
            qApp->aboutQt();
        } else
            QDesktopServices::openUrl(QUrl(link));
    };

    QString about = ui.about_text->text();
    about.replace("___versionstring___", qApp->applicationVersion());
    ui.about_text->setText(about);
    connect(ui.about_text, &QLabel::linkActivated, this, open_link);
}

void SettingsWindow::bringToFront()
{
    show();
    raise();
    activateWindow();
}

void SettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Escape)
        close();
    else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_W)
        close();
}
