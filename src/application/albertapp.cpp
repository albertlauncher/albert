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
#include <csignal>
#include "albertapp.h"
#include "mainwindow.h"
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
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
#endif
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
        exit(1);
    }
}


/** ***************************************************************************/
AlbertApp::AlbertApp(int &argc, char *argv[]) : QApplication(argc, argv) {

    /*
     *  INITIALIZE APPLICATION
     */

    qInstallMessageHandler(myMessageOutput);
    setOrganizationDomain("albert");
    setApplicationName("albert");
    setApplicationDisplayName("Albert");
    setApplicationVersion("v0.8.7");
    setWindowIcon(QIcon(":app_icon"));
    setQuitOnLastWindowClosed(false);


    /*
     * INITIALISATION
     */

    // Make sure data, cache and config dir exists
    QDir dir;
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + applicationName());
    dir.mkpath(".");
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    dir.mkpath(".");
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    dir.mkpath(".");

    bool performFullSetup = true;

    // Check if running-indicator-file exists
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running";
    if (QFile::exists(filePath)) {
        // Yep, it does!
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Could not open pid-file: " << filePath;
            file.close();
        } else {
            // Extracting PID from indicator file
            qint64 pid = -1;
            file.read((char*) &pid, sizeof(qint64));
            file.close();
            if (pid == -1)
                qCritical() << "This failed though!";
            else {
                // Send the process a SIGUSR1 to bring it to front.
                int result = kill(pid, SIGUSR1);
                if (result == 0) {
                    // The signal was correctly delivered. This means the
                    //     process is running.
                    // Set the flag to indicate that the application should not
                    //     perform a setup
                    performFullSetup = false;
                } else {
                    // The signal was not correctly delivered. This means the
                    //     process is not running.
                    // Therefore print the error message
                    qCritical() << "Application has not been terminated graciously";
                    QSettings s;
                    if (s.value("warnAboutNonGraciousQuit") != false){
                        QMessageBox msgBox(QMessageBox::Critical, "Error",
                                           "Albert has not been quit graciously! This "
                                           "means your settings and data have not been "
                                           "saved. If you did not kill albert yourself, "
                                           "albert most likely crashed. Please report this "
                                           "on github. Do you want to ignore this warnings "
                                           "in future?",
                                           QMessageBox::Yes|QMessageBox::No);
                        msgBox.exec();
                        if ( msgBox.result() == QMessageBox::Yes )
                            s.setValue("warnAboutNonGraciousQuit", false);
                    }

                    // But the file has still the wrong PID
                    // Update it!
                    writePidFile(filePath);
                }
            }
        }
    } else {
        // Create the running indicator file
        writePidFile(filePath);
    }

    // Check if a full setup should be performed
    if (performFullSetup) {
        // And do so

        mainWindow_ = new MainWindow;
        hotkeyManager_ = new HotkeyManager;
        pluginManager_ = new PluginManager;
        extensionManager_ = new ExtensionManager;

        // Propagade the extensions once
        for (const unique_ptr<PluginSpec> &p : pluginManager_->plugins())
            if (p->isLoaded())
                extensionManager_->registerExtension(p->instance());

        // Quit gracefully on SIGTERM
        signal(SIGTERM, [](int){
            QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
        });

        // Bring the widget to front when receiving SIGUSR1
        signal(SIGUSR1, [](int){
            QMetaObject::invokeMethod(qApp, "showWidget", Qt::QueuedConnection);
        });

        /*
         *  SETUP SIGNAL FLOW
         */

        // Show mainwindow if hotkey is pressed
        QObject::connect(hotkeyManager_, &HotkeyManager::hotKeyPressed,mainWindow_, &MainWindow::toggleVisibility);

        // Extrension manager signals new proposals
        QObject::connect(extensionManager_, &ExtensionManager::newModel, mainWindow_, &MainWindow::setModel);

        // Setup and teardown query sessions with the state of the widget
        QObject::connect(mainWindow_, &MainWindow::widgetShown,  extensionManager_, &ExtensionManager::setupSession);
        QObject::connect(mainWindow_, &MainWindow::widgetHidden, extensionManager_, &ExtensionManager::teardownSession);

        // A change in text triggers requests
        QObject::connect(mainWindow_, &MainWindow::startQuery, extensionManager_, &ExtensionManager::startQuery);

        // Publish loaded plugins to the specific interface handlers
        QObject::connect(pluginManager_, &PluginManager::pluginLoaded, extensionManager_, &ExtensionManager::registerExtension);
        QObject::connect(pluginManager_, &PluginManager::pluginAboutToBeUnloaded, extensionManager_, &ExtensionManager::unregisterExtension);

        this->fullySetup = true;
    } else {
        this->fullySetup = false;
    }
}



/** ***************************************************************************/
AlbertApp::~AlbertApp() {

    /*
     *  FINALIZE APPLICATION
     */

    // Only attempt to delete this when the application has performed a full setup
    // else this will fail because the objects are not instantiated.
    // Also the running-file should not be deleted
    if (this->fullySetup) {
        // Unload the plugins
        delete extensionManager_;
        delete pluginManager_;
        delete hotkeyManager_;
        delete mainWindow_;

        // Delete the running indicator file
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running");
    }
}



/** ***************************************************************************/
void AlbertApp::writePidFile(QString &filename) {
    QFile pidFile(filename);
    if (pidFile.open(QFile::WriteOnly)) {
        qint64 pid = applicationPid();
        int wrote = pidFile.write((char*) &pid, sizeof(qint64));
        if (wrote != sizeof(qint64)) {
            qCritical("Failed to write pidfile!");
        }
    } else {
        qCritical("Could not open pidfile for write!");
    }
    pidFile.close();
}



/** ***************************************************************************/
int AlbertApp::exec() {
    if (!this->fullySetup) {
        // There was not full setup performed...
        // This means that this is a secondary instance
        return EXIT_SUCCESS;
    }
    //  HOTKEY  //  Albert without hotkey is useless. Force it!
    QSettings s;
    QVariant v;
    if (!(s.contains("hotkey") && (v=s.value("hotkey")).canConvert(QMetaType::QString)
            && hotkeyManager_->registerHotkey(v.toString()))){
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
    if (!settingsWidget_)
        settingsWidget_ = new SettingsWidget(mainWindow_, hotkeyManager_, pluginManager_);
    settingsWidget_->show();
    settingsWidget_->raise();
}



/** ***************************************************************************/
void AlbertApp::showWidget() {
    mainWindow_->show();
}



/** ***************************************************************************/
void AlbertApp::hideWidget() {
    mainWindow_->hide();
}
