// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extension/frontend/frontend.h"
#include "albert/logging.h"
#include "app.h"
#include "pluginwidget.h"
#include "qtpluginloader.h"
#include "settings/globalsearchwidget.h"
#include "settingswindow.h"
#include "trayicon.h"
#include "triggerwidget.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QKeySequenceEdit>
#include <QStandardItemModel>
#include <QStandardPaths>
using namespace std;

enum class Tab {
    General,
    Window,
    Search,
    Triggers,
    Plugins,
    About
};

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
    init_tabs(app);

    auto geometry = QGuiApplication::screenAt(QCursor::pos())->geometry();
    move(geometry.center().x() - frameSize().width()/2,
         geometry.top() + geometry.height() / 5);
}

void SettingsWindow::init_tabs(App &app)
{
    ui.tabs->setStyleSheet("QTabWidget::pane { border-radius: 0px; }");

    // Tab 0 general
    init_tab_general_hotkey(app);
    init_tab_general_trayIcon(app);
    init_tab_general_autostart();
    init_tab_general_frontends(app);
    init_tab_general_terminals(app);

    // Tab 1 Window
    ui.tabs->insertTab((int)Tab::Window, app.frontend->createFrontendConfigWidget(), "Window");

    // Tab 2 Search
    ui.tabs->insertTab((int)Tab::Search, new GlobalSearchWidget(app.query_engine, app.extension_registry), "Search");

    // Tab 3 Triggers
    ui.tabs->insertTab((int)Tab::Triggers, new TriggerWidget(app.query_engine, app.extension_registry), "Triggers");

    // Tab 4 Plugins
    ui.tabs->insertTab((int)Tab::Plugins, new PluginWidget(app.plugin_registry), "Plugins");

    // Tab 5 About
    init_tab_about();
}

void SettingsWindow::init_tab_general_hotkey(App &app)
{
    ui.formLayout_general->insertRow(0, "Hotkey", new QHotKeyEdit(app.hotkey));
}

void SettingsWindow::init_tab_general_trayIcon(App &app)
{
    ui.checkBox_showTray->setChecked(app.tray_icon.isVisible());
    QObject::connect(ui.checkBox_showTray, &QCheckBox::toggled,
                     &app.tray_icon, &TrayIcon::setVisible);
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

void SettingsWindow::init_tab_general_frontends(App &app)
{
    // Populate frontend checkbox
    for (const auto *loader : app.plugin_provider.frontendPlugins()){
        ui.comboBox_frontend->addItem(loader->metaData().name, loader->metaData().id);
        if (loader->metaData().id == app.frontend->id())
            ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
    }
    connect(ui.comboBox_frontend, &QComboBox::currentIndexChanged, this, [this, &app]() {
        app.setFrontend(ui.comboBox_frontend->currentData().toString());
    });

}

void SettingsWindow::init_tab_general_terminals(App &app)
{
    for (const auto &terminal : app.terminal_provider.terminals()){
        ui.comboBox_term->addItem(terminal->name());
        if (terminal.get() == &app.terminal_provider.terminal())
            ui.comboBox_term->setCurrentIndex(ui.comboBox_term->count()-1);
    }

    connect(ui.comboBox_term, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [&app](int index){ app.terminal_provider.setTerminal(index); });

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
}
