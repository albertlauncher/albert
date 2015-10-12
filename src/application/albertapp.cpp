// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include "albertapp.h"
#include "mainwidget.h"
#include "settingswidget.h"
#include "hotkeymanager.h"
#include "pluginmanager.h"
#include "extensionmanager.h"


/** ***************************************************************************/
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message) {
    QString suffix;
    if (context.function)
        suffix = QString("  --  [%1]").arg(context.function);
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "%s\n", message.toLocal8Bit().constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "\x1b[33;1mWarning:\x1b[0;1m %s%s\x1b[0m\n", message.toLocal8Bit().constData(), suffix.toLocal8Bit().constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "\x1b[31;1mCritical:\x1b[0;1m %s%s\x1b[0m\n", message.toLocal8Bit().constData(), suffix.toLocal8Bit().constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "\x1b[41;30;4mFATAL:\x1b[0;1m %s%s\x1b[0m\n", message.toLocal8Bit().constData(), suffix.toLocal8Bit().constData());
        abort();
    }
}


/** ***************************************************************************/
AlbertApp::AlbertApp(int &argc, char *argv[]) : QApplication(argc, argv) {
    /*
     *  INITIALIZE APPLICATION
     */

    qInstallMessageHandler(myMessageOutput);
    setOrganizationDomain("manuelschneid3r");
    setApplicationName("albert");
    setApplicationDisplayName("Albert");
    setApplicationVersion("v0.7.4");
    setWindowIcon(QIcon(":app_icon"));
    setQuitOnLastWindowClosed(false); // Dont quit after settings close


    /*
     * INITIALISATION
     */

    // MAKE SURE THE NEEDED DIRECTORIES EXIST
    QDir data(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if (!data.exists())
        data.mkpath(".");

    _mainWidget = new MainWidget;
    _hotkeyManager = new HotkeyManager;
    _pluginManager = new PluginManager;
    _extensionManager = new ExtensionManager;

    // Propagade the extensions once
    for (const unique_ptr<PluginSpec> &p : _pluginManager->plugins())
        if (p->isLoaded())
            _extensionManager->registerExtension(p->instance());


    /*
     *  SETUP SIGNAL FLOW
     */

    // Show mainwidget if hotkey is pressed
    QObject::connect(_hotkeyManager, &HotkeyManager::hotKeyPressed,_mainWidget, &MainWidget::toggleVisibility);

    // Extrension manager signals new proposals
    QObject::connect(_extensionManager, &ExtensionManager::newModel, _mainWidget, &MainWidget::setModel);

    // Setup and teardown query sessions with the state of the widget
    QObject::connect(_mainWidget, &MainWidget::widgetShown,  _extensionManager, &ExtensionManager::setupSession);
    QObject::connect(_mainWidget, &MainWidget::widgetHidden, _extensionManager, &ExtensionManager::teardownSession);
    // Click on _settingsButton (or shortcut) closes albert + opens settings dialog
    QObject::connect(_mainWidget->ui.inputLine->_settingsButton, &QPushButton::clicked, _mainWidget, &MainWidget::hide);
    QObject::connect(_mainWidget->ui.inputLine->_settingsButton, &QPushButton::clicked, this, &AlbertApp::openSettings);
    // A change in text triggers requests
    QObject::connect(_mainWidget->ui.inputLine, &InputLine::textChanged, _extensionManager, &ExtensionManager::startQuery);
    // Enter triggers action
    QObject::connect(_mainWidget->ui.proposalList, &ProposalList::activated, _extensionManager, &ExtensionManager::activate);

    // Publish loaded plugins to the specific interface handlers
    QObject::connect(_pluginManager, &PluginManager::pluginLoaded, _extensionManager, &ExtensionManager::registerExtension);
    QObject::connect(_pluginManager, &PluginManager::pluginAboutToBeUnloaded, _extensionManager, &ExtensionManager::unregisterExtension);

    // Hide on focus loss
//    QObject::connect(this, &QApplication::applicationStateChanged, this, &AlbertApp::onStateChange);


    // TESTING AREA

}



/** ***************************************************************************/
AlbertApp::~AlbertApp() {
    // Unload the plugins
    delete _extensionManager;
    delete _pluginManager;
    delete _hotkeyManager;
    delete _mainWidget;
}



/** ***************************************************************************/
int AlbertApp::exec() {
    //  HOTKEY  //  Albert without hotkey is useless. Force it!
    QSettings s;
    QVariant v;
    if (!(s.contains("hotkey") && (v=s.value("hotkey")).canConvert(QMetaType::QString)
            && _hotkeyManager->registerHotkey(v.toString()))){
        QMessageBox msgBox(QMessageBox::Critical, "Error",
                           "Hotkey is not set or invalid. Press ok to open "
                           "the settings or press close to quit albert.",
                           QMessageBox::Close|QMessageBox::Ok);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ) {
            //hotkeyManager->disable();
            openSettings();
            //QObject::connect(settingsWidget, &QWidget::destroyed, hotkeyManager, &HotkeyManager::enable);
        }
        else
            return EXIT_FAILURE;
    }
    return QApplication::exec();
}



/** ***************************************************************************/
void AlbertApp::openSettings() {
    if (!_settingsWidget)
        _settingsWidget = new SettingsWidget(_mainWidget, _hotkeyManager, _pluginManager);
    _settingsWidget->show();
}



/** ***************************************************************************/
void AlbertApp::showWidget() {
    _mainWidget->show();
}



/** ***************************************************************************/
void AlbertApp::hideWidget() {
    _mainWidget->hide();
}



///** ***************************************************************************/
//void AlbertApp::onStateChange(Qt::ApplicationState state) {
//    if (state==Qt::ApplicationInactive)
//        _mainWidget->hide();
//}
