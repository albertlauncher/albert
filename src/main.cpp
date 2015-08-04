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

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include <QFileSystemWatcher>

#include <functional>

#include "mainwidget.h"
#include "settingswidget.h"
#include "hotkeymanager.h"
#include "pluginhandler.h"
#include "extensionhandler.h"

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

int main(int argc, char *argv[])
{

	/*
	 *  INITIALIZE APPLICATION
	 */

    qInstallMessageHandler(myMessageOutput);
    QApplication application(argc, argv);
    application.setOrganizationDomain("manuelschneid3r");
    application.setApplicationName("albert");
    application.setApplicationDisplayName("Albert");
    application.setApplicationVersion("0.6");
	application.setWindowIcon(QIcon(":app_icon"));
	application.setQuitOnLastWindowClosed(false); // Dont quit after settings close

    MainWidget *mainWidget = new MainWidget;
    HotkeyManager *hotkeyManager = new HotkeyManager;
    PluginHandler *pluginHandler = new PluginHandler;
    ExtensionHandler *extensionHandler = new ExtensionHandler;

    std::function<SettingsWidget*()> openSettings = [=](){
        SettingsWidget *sw = new SettingsWidget(mainWidget, hotkeyManager, pluginHandler);
        sw->show();
        return sw;
    };



    /*
     * INITIALISATION
     */

    {
        QSettings s;

        // MAKE SURE THE NEEDED DIRECTORIES EXIST
        QDir data(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        if (!data.exists())
            data.mkpath(".");
        QDir conf(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                  +"/"+ qApp->applicationName());
        if (!conf.exists())
            conf.mkpath(".");


        //  HOTKEY  //  Albert without hotkey is useless. Force it!
        hotkeyManager->registerHotkey(s.value("hotkey").toString());
        if (hotkeyManager->hotkeys().empty()) {
            QMessageBox msgBox(QMessageBox::Critical, "Error",
                               "Hotkey is not set or invalid. Press ok to open "
                               "the settings or press close to quit albert.",
                               QMessageBox::Close|QMessageBox::Ok);
            msgBox.exec();
            if ( msgBox.result() == QMessageBox::Ok ){
                hotkeyManager->disable();
                SettingsWidget *sw = openSettings();
                QObject::connect(sw, &QWidget::destroyed, hotkeyManager, &HotkeyManager::enable);
                sw->ui.tabs->setCurrentIndex(0);
                sw->show();
            }
            else
                exit(0);
        }
    }



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
    QObject::connect(mainWidget->ui.inputLine->_settingsButton, &QPushButton::clicked, openSettings);

    // A change in text triggers requests
    QObject::connect(mainWidget->ui.inputLine, &QLineEdit::textChanged, extensionHandler, &ExtensionHandler::startQuery);

    // Publish loaded plugins to the specific interface handlers
    QObject::connect(pluginHandler, &PluginHandler::pluginLoaded, extensionHandler, &ExtensionHandler::registerExtension);
    QObject::connect(pluginHandler, &PluginHandler::pluginAboutToBeUnloaded, extensionHandler, &ExtensionHandler::unregisterExtension);



    /*
     * START THE ALBERT MACHINERY
     */

    mainWidget->ui.proposalList->setModel(extensionHandler); // View results
    pluginHandler->loadPlugins(); // Load the plugins
    int ret = application.exec();
    pluginHandler->unloadPlugins(); // Unload the plugins



    /*
     *  CLEANUP
     */

    delete hotkeyManager;
    delete extensionHandler;
    delete pluginHandler;
    delete mainWidget;
    return ret;
}
