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

#include "albertapp.h"
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>

/** ***************************************************************************/
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QString suffix;
    if (context.function)
        suffix = QString("  --  [%1]").arg(context.function);
    switch (type) {
    case QtInfoMsg:
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
AlbertApp::AlbertApp(int &argc, char *argv[]) : QApplication(argc, argv)
{
    /*
     *  INITIALIZE APPLICATION
     */

    qInstallMessageHandler(myMessageOutput);
    setOrganizationDomain("manuelschneid3r");
    setApplicationName("albert");
    setApplicationDisplayName("Albert");
    setApplicationVersion("0.6");
    setWindowIcon(QIcon(":app_icon"));
    setQuitOnLastWindowClosed(false); // Dont quit after settings close

    mainWidget = new MainWidget;
    hotkeyManager = new HotkeyManager;
    pluginHandler = new PluginHandler;
    extensionHandler = new ExtensionHandler;


    /*
     * INITIALISATION
     */

    // View results
    mainWidget->ui.proposalList->setModel(extensionHandler);


    // MAKE SURE THE NEEDED DIRECTORIES EXIST
    QDir data(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if (!data.exists())
        data.mkpath(".");
    QDir conf(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
              +"/"+ qApp->applicationName());
    if (!conf.exists())
        conf.mkpath(".");


    /*
     *  SETUP SIGNAL FLOW
     */

    // Show mainwidget if hotkey is pressed
    QObject::connect(hotkeyManager, &HotkeyManager::hotKeyPressed,mainWidget, &MainWidget::toggleVisibility);

    // Setup and teardown query sessions with the state of the widget
    QObject::connect(mainWidget, &MainWidget::widgetShown, extensionHandler, &ExtensionHandler::setupSession);
    QObject::connect(mainWidget, &MainWidget::widgetHidden, extensionHandler, &ExtensionHandler::teardownSession);

    // Click on _settingsButton (or shortcut) closes albert + opens settings dialog
    QObject::connect(mainWidget->ui.inputLine->_settingsButton, &QPushButton::clicked, mainWidget, &MainWidget::hide);
    QObject::connect(mainWidget->ui.inputLine->_settingsButton, &QPushButton::clicked, this, &AlbertApp::openSettings);

    // A change in text triggers requests
    QObject::connect(mainWidget->ui.inputLine, &InputLine::textChanged, extensionHandler, &ExtensionHandler::startQuery);

    // Enter triggers action
    QObject::connect(mainWidget->ui.proposalList, &ProposalList::activated, extensionHandler, &ExtensionHandler::activate);

    // Publish loaded plugins to the specific interface handlers
    QObject::connect(pluginHandler, &PluginHandler::pluginLoaded, extensionHandler, &ExtensionHandler::registerExtension);
    QObject::connect(pluginHandler, &PluginHandler::pluginAboutToBeUnloaded, extensionHandler, &ExtensionHandler::unregisterExtension);

    // Hide on focus loss
    QObject::connect(this, &QApplication::applicationStateChanged, this, &AlbertApp::onStateChange);

    // Load the plugins
    pluginHandler->loadPlugins();

    // TESTING AREA

}



/** ***************************************************************************/
AlbertApp::~AlbertApp()
{
    // Unload the plugins
    pluginHandler->unloadPlugins();
    delete hotkeyManager;
    delete extensionHandler;
    delete pluginHandler;
    delete mainWidget;
}



/** ***************************************************************************/
int AlbertApp::exec()
{
    //  HOTKEY  //  Albert without hotkey is useless. Force it!
    QSettings s;
    hotkeyManager->registerHotkey(s.value("hotkey").toString());
    if (hotkeyManager->hotkeys().empty()) {
        QMessageBox msgBox(QMessageBox::Critical, "Error",
                           "Hotkey is not set or invalid. Press ok to open "
                           "the settings or press close to quit albert.",
                           QMessageBox::Close|QMessageBox::Ok);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ){
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
void AlbertApp::openSettings()
{
    settingsWidget = new SettingsWidget(mainWidget, hotkeyManager, pluginHandler);
    settingsWidget->show();
}



/** ***************************************************************************/
void AlbertApp::onQuit()
{
    qDebug() << "!onQuit";
}



/** ***************************************************************************/
void AlbertApp::onStateChange(Qt::ApplicationState state)
{
    if (state==Qt::ApplicationInactive)
        mainWidget->hide();
}
