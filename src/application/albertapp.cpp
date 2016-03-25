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
    setApplicationVersion("v0.8.5");
    setWindowIcon(QIcon(":app_icon"));
    setQuitOnLastWindowClosed(false);


    /*
     * INITIALISATION
     */

    // Make sure data, cache and config dir exists
    QDir dir;
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/"+applicationName());
    dir.mkpath(".");
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    dir.mkpath(".");
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    dir.mkpath(".");

    // Print e message if the app was not terminated graciously
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running";
    if (QFile::exists(filePath))
        qCritical() << "Application has not been terminated graciously";
    else {
        // Create the running indicator file
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
            qCritical() << "Could not create file:" << filePath;
        file.close();
    }

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
}



/** ***************************************************************************/
AlbertApp::~AlbertApp() {

    /*
     *  FINALIZE APPLICATION
     */
    // Unload the plugins
    delete extensionManager_;
    delete pluginManager_;
    delete hotkeyManager_;
    delete mainWindow_;

    // Delete the running indicator file
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running");
}



/** ***************************************************************************/
int AlbertApp::exec() {
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
