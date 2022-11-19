// Copyright (c) 2022 Manuel Schneider

#include "albert/extensionregistry.h"
#include "albert/logging.h"
#include "pluginprovider.h"
#include "pluginwidget.h"
#include "queryengine.h"
#include "configproviderwidget.h"
#include "triggerwidget.h"
#include "settingswindow.h"
#include "terminalprovider.h"
#include <QCloseEvent>
#include <QDesktopServices>
using namespace std;

SettingsWindow::SettingsWindow(albert::ExtensionRegistry &er,
                               PluginProvider &pp,
                               QueryEngine &qe,
                               TerminalProvider &tp) : ui()
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui.tabs->setStyleSheet("QTabWidget::pane { border-radius: 0px; }");

    init_tab_general_frontend(pp);
    init_tab_general_terminal(tp);
    init_tab_general_trayIcon();
    init_tab_general_autostart();
    init_tab_general_fuzzy(qe);
    init_tab_general_separators(qe);
    ui.tabs->insertTab(ui.tabs->count()-1, pp.frontend->createSettingsWidget(), tr("Frontend"));
    ui.tabs->insertTab(ui.tabs->count()-1, new ConfigProviderWidget(er), tr("Extensions"));
    ui.tabs->insertTab(ui.tabs->count()-1, new TriggerWidget(qe), "Triggers");
    ui.tabs->insertTab(ui.tabs->count()-1, new PluginWidget(er), "Plugins");
    init_tab_about();

    auto geometry = QGuiApplication::screenAt(QCursor::pos())->geometry();
    move(geometry.center().x() - frameSize().width()/2,
         geometry.top() + geometry.height() / 5);
}


void SettingsWindow::init_tab_general_frontend(PluginProvider &plugin_provider)
{
    for (const auto &spec : plugin_provider.frontends()){
        ui.comboBox_frontend->addItem(spec.name);
        if (spec.id == plugin_provider.frontend->id())
            ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
    }

    connect(ui.comboBox_frontend, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [&plugin_provider](int index) { plugin_provider.setFrontend(index); });
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

void SettingsWindow::init_tab_general_trayIcon()
{
//    ui.checkBox_showTray->setChecked(albert.tray_icon.isVisible());
//    QObject::connect(ui.checkBox_showTray, &QCheckBox::toggled,
//                     &albert.tray_icon, &TrayIcon::setVisible);
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

void SettingsWindow::init_tab_general_fuzzy(QueryEngine &engine)
{
    ui.checkBox_fuzzy->setChecked(engine.fuzzy());
    QObject::connect(ui.checkBox_fuzzy, &QCheckBox::toggled,
                     [&engine](bool checked){ engine.setFuzzy(checked); });
}

void SettingsWindow::init_tab_general_separators(QueryEngine &engine)
{
    ui.lineEdit_separators->setText(engine.separators());
    QObject::connect(ui.lineEdit_separators, &QLineEdit::editingFinished,
                     [&](){ engine.setSeparators(ui.lineEdit_separators->text()); });
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
}





//void SettingsWindow::changeHotkey(int /*newhk*/)
//{
//    Q_ASSERT(hotkeyManager_);
//    int oldhk = *hotkeyManager_->hotkeys().begin(); // TODO Make cool sharesdpointer design
//    // Try to set the hotkey
//    if (hotkeyManager_->registerHotkey(newhk)) {
//        QString hkText(QKeySequence((newhk&~Qt::GroupSwitchModifier)).toString());//QTBUG-45568
//        ui.grabKeyButton_hotkey->setText(hkText);
//        QSettings().setValue("hotkey", hkText);
//        hotkeyManager_->unregisterHotkey(oldhk);
//    } else {
//        ui.grabKeyButton_hotkey->setText(QKeySequence(oldhk).toString());
//        QMessageBox(QMessageBox::Critical, "Error",
//                    QKeySequence(newhk).toString() + " could not be registered.",
//                    QMessageBox::NoButton,
//                    this).exec();
//    }
//}
//void SettingsWindow::closeEvent(QCloseEvent */*event*/)
//{
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
//}
//void SettingsWindow::init_tab_general_hotkey()
//{
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
//}

