// Copyright (c) 2022 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "pluginwidget.h"
#include "settingswidget.h"
#include "settingswindow.h"
#include <QCloseEvent>
#include <QDesktopServices>
using namespace std;
using namespace albert;

SettingsWindow::SettingsWindow(App &app) : app(app), ui()
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui.tabs->setStyleSheet("QTabWidget::pane { border-radius: 0px; }");

    init_tab_general_hotkey();
    init_tab_general_frontend();
    init_tab_general_terminal();
    init_tab_general_trayIcon();
    init_tab_general_about();

    auto *settings_widget = new SettingsWidget(app);
    ui.tabs->addTab(settings_widget, "Settings");

    auto plugin_widget = new PluginWidget();
    ui.tabs->addTab(plugin_widget, "Plugins");

//    auto stats_widget = new StatsWidget;
//    ui.tabs->addTab(stats_widget, "Stats");

    auto geometry = QGuiApplication::screenAt(QCursor::pos())->geometry();
    move(geometry.center().x() - frameSize().width()/2,
         geometry.top() + geometry.height() / 5);
}

void SettingsWindow::init_tab_general_hotkey()
{
//    // HOTKEY
//    if (hotkeyManager) {
//        QSet<int> hks = hotkeyManager->hotkeys();
//        if (hks.size() < 1)
//            ui.grabKeyButton_hotkey->setText("Press to set hotkey");
//        else
//            ui.grabKeyButton_hotkey->setText(QKeySequence(*hks.begin()).toString()); // OMG
//        connect(ui.grabKeyButton_hotkey, &GrabKeyButton::keyCombinationPressed,
//                this, &SettingsWindow::changeHotkey);
//    } else {
//        ui.grabKeyButton_hotkey->setVisible(false);
//        ui.label_hotkey->setVisible(false);
    //    }
}

void SettingsWindow::init_tab_general_frontend()
{
    for (const auto &[id, spec] : app.plugin_provider.plugins()){
        if (spec.type != PluginType::Frontend)
            continue;

        // Add item (text and id)
        ui.comboBox_frontend->addItem(spec.name, id);

        // Set to current if ids match
        if (spec.state == albert::PluginState::Loaded)
            ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
    }

    auto *frontend_settings_widget = app.frontend->createSettingsWidget();
    ui.tabs->addTab(frontend_settings_widget, "Frontend");

    connect(ui.comboBox_frontend, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        QString id = ui.comboBox_frontend->itemData(index, Qt::UserRole).toString();
        app.setFrontend(id);

        if (id == app.frontend->id()){
            QMessageBox::StandardButton reply =
                    QMessageBox::question(ui.tabGeneral, "Restart?",
                                          "Changing the frontend needs a restart. Do you want to restart Albert?",
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (reply == QMessageBox::Yes)
                albert::restart();
        }
    });
}

void SettingsWindow::init_tab_general_terminal()
{
    for (const auto &terminal : app.terminal_provider.terminals()){
        ui.comboBox_term->addItem(terminal->name());
        if (terminal.get() == &app.terminal_provider.terminal())
            ui.comboBox_term->setCurrentIndex(ui.comboBox_term->count()-1);
    }

    connect(ui.comboBox_term, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){ app.terminal_provider.setTerminal(index); });
}

void SettingsWindow::init_tab_general_trayIcon()
{
//    ui.checkBox_showTray->setChecked(albert.tray_icon.isVisible());
//    QObject::connect(ui.checkBox_showTray, &QCheckBox::toggled,
//                     &albert.tray_icon, &TrayIcon::setVisible);
}

void SettingsWindow::init_tab_general_about()
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

void SettingsWindow::changeHotkey(int /*newhk*/)
{
//    Q_ASSERT(hotkeyManager_);
//    int oldhk = *hotkeyManager_->hotkeys().begin(); // TODO Make cool sharesdpointer design

//    // Try to set the hotkey
//    if (hotkeyManager_->registerHotkey(newhk)) {
//        QString hkText(QKeySequence((newhk&~Qt::GroupSwitchModifier)).toString());//QTBUG-45568
//        ui.grabKeyButton_hotkey->setText(hkText);
//        QSettings(qApp->applicationName()).setValue("hotkey", hkText);
//        hotkeyManager_->unregisterHotkey(oldhk);
//    } else {
//        ui.grabKeyButton_hotkey->setText(QKeySequence(oldhk).toString());
//        QMessageBox(QMessageBox::Critical, "Error",
//                    QKeySequence(newhk).toString() + " could not be registered.",
//                    QMessageBox::NoButton,
//                    this).exec();
//    }
}

void SettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Escape)
        close();
}

void SettingsWindow::closeEvent(QCloseEvent */*event*/)
{
//    if (hotkeyManager_ && hotkeyManager_->hotkeys().empty()) {
//        QMessageBox msgBox(QMessageBox::Warning, "Hotkey Missing",
//                           "Hotkey is invalid, please set it. Press OK to go "\
//                           "back to the settings.",
//                           QMessageBox::Ok|QMessageBox::Ignore,
//                           this);
//        msgBox.exec();
//        if ( msgBox.result() == QMessageBox::Ok ) {
//            ui.tabs->setCurrentIndex(0);
//            show();
//            event->ignore();
//            return;
//        }
//    }
    //    event->accept();
}







//void SettingsWindow::initializeSettings_autostart()
//{
//    #if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
//    QString desktopfile_path = QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
//                                                      "albert.desktop",
//                                                      QStandardPaths::LocateFile);
//    if (!desktopfile_path.isNull()) {
//        QString autostart_path = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).filePath("autostart/albert.desktop");
//        ui.checkBox_autostart->setChecked(QFile::exists(autostart_path));
//        connect(ui.checkBox_autostart, &QCheckBox::toggled,
//                this, [=](bool toggled){
//            if (toggled)
//                QFile::link(desktopfile_path, autostart_path);
//            else
//                QFile::remove(autostart_path);
//        });
//    }
//    else
//        CRIT << "Deskop entry not found! Autostart option is nonfuctional";
//    #else
//    ui.checkBox_autostart->setEnabled(false);
//    WARN << "Autostart not implemented on this platform!";
//    #endif
//}

