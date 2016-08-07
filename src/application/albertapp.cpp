// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QCommandLineParser>
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>
#include <QDebug>
#include <QMenu>
#include <csignal>
#include "albertapp.h"
#include "mainwindow.h"
#include "hotkeymanager.h"
#include "extensionmanager.h"
#include "settingswidget.h"
#include "abstractextension.h"
#include "query_p.h"

const char* AlbertApp::CFG_TERM = "terminal";
const char* AlbertApp::DEF_TERM = "xterm -e %1";
const char* AlbertApp::CFG_SHOWTRAY = "showTray";
const bool  AlbertApp::DEF_SHOWTRAY = true;

namespace {

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

void shutdownHandler(int) {
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}
}

/** ***************************************************************************/
AlbertApp::AlbertApp(int &argc, char *argv[])
    : QApplication(argc, argv),
      mainWindow_(nullptr),
      hotkeyManager_(nullptr),
      extensionManager_(nullptr),
      currentQuery_(nullptr),
      localServer_(nullptr),
      settings_(nullptr),
      trayIcon_(nullptr),
      trayIconMenu_(nullptr) {


    /*
     *  INITIALIZE APPLICATION
     */


    qInstallMessageHandler(myMessageOutput);
    setOrganizationDomain("albert");
    setApplicationName("albert");
    setApplicationDisplayName("Albert");
    setApplicationVersion("v0.8.10");
    setWindowIcon(QIcon(":app_icon"));
    setQuitOnLastWindowClosed(false);



    /*
     *  PARSE COMMANDLINE
     */


    QCommandLineParser parser;
    parser.setApplicationDescription("Albert is still in alpha. These options "
                                     "may change in future versions.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption({"c", "config"}, "The config file to use.", "file"));
    parser.addOption(QCommandLineOption({"k", "hotkey"}, "Overwrite the hotkey to use.", "hotkey"));
    parser.addPositionalArgument("command", "Command to send to a running instance, if any. (show, hide, toggle)", "[command]");
    parser.process(*this);

    const QStringList args = parser.positionalArguments();
    if ( args.count() > 1)
        qFatal("Invalid amount of arguments");

    if ( parser.isSet("config") ) {
        settings_ = new QSettings(parser.value("c"));
    } else {
        settings_ = new QSettings;
    }

    if ( parser.isSet("hotkey") ) {
        settings_->setValue("hotkey", parser.value("hotkey"));
    }



    /*
     *  SINGLE INSTANCE / IPC
     */


    QLocalSocket socket;
    socket.connectToServer(applicationName());
    if ( socket.waitForConnected(500) ) {
        // If there is a command send it
        if ( args.count() == 1 ){
            socket.write(args.at(0).toLocal8Bit());
            socket.flush();
            socket.waitForReadyRead(500);
            if (socket.bytesAvailable())
                qDebug() << socket.readAll();
        }
        else
            qDebug("There is another instance of albert running.");
        socket.close();
        ::exit(EXIT_SUCCESS);
    } else if ( args.count() == 1 ) {
        qDebug("There is no other instance of albert running.");
        ::exit(EXIT_FAILURE);
    }

    // Start server so second instances will close
    QLocalServer::removeServer(applicationName());
    localServer_ = new QLocalServer(this);
    localServer_->listen(applicationName());
    QObject::connect(localServer_, &QLocalServer::newConnection, [this] () {
        QLocalSocket* socket = localServer_->nextPendingConnection(); // Should be safe
        socket->waitForReadyRead(500);
        if (socket->bytesAvailable()) {
            QString msg = QString::fromLocal8Bit(socket->readAll());
            if ( msg == "show") {
                mainWindow_->show();
                socket->write("Application set visible.");
            } else if ( msg == "hide") {
                mainWindow_->hide();
                socket->write("Application set invisible.");
            } else if ( msg == "toggle") {
                mainWindow_->setVisible(!mainWindow_->isVisible());
                socket->write("Visibility toggled.");
            } else
                socket->write("Command not supported.");
        }
        socket->flush();
        socket->close();
        socket->deleteLater();
    });


    /*
     * INITIALIZE PATHS
     */

    // Make sure data, cache and config dir exists
    QDir dir;
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + applicationName());
    dir.mkpath(".");
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    dir.mkpath(".");
    dir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    dir.mkpath(".");



    /*
     * INITIALIZE APPLICATION
     */


    mainWindow_ = new MainWindow;
    hotkeyManager_ = new HotkeyManager;
    extensionManager_ = new ExtensionManager;

    // toggle visibility
    QObject::connect(hotkeyManager_, &HotkeyManager::hotKeyPressed,[this](){
        mainWindow_->setVisible(!mainWindow_->isVisible());
    });

    QObject::connect(mainWindow_, &MainWindow::widgetShown,
                     this, &AlbertApp::onWidgetShown);

    QObject::connect(mainWindow_, &MainWindow::widgetHidden,
                     this, &AlbertApp::onWidgetHidden);

    QObject::connect(mainWindow_, &MainWindow::inputChanged,
                     this, &AlbertApp::onInputChanged);

    // Enable the tray icon
    QVariant v = qApp->settings()->value(CFG_SHOWTRAY, DEF_SHOWTRAY);
    if (v.isValid() && v.canConvert(QMetaType::Bool))
        enableTrayIcon(v.toBool());



    /*
     *  UNIX STUFF THAT SHOULD BE IN A PIMPL OR SUBCLASS
     */


    // Set terminal emulator
    v = qApp->settings()->value(CFG_TERM);
    if (v.isValid() && v.canConvert(QMetaType::QString))
        terminal_ = v.toString();
    else{
        terminal_ = getenv("TERM");
        if (terminal_.isEmpty())
            terminal_ = DEF_TERM;
        else
            terminal_.append(" -e %1");
    }

    // Quit gracefully on unix signals
    for ( int sig : {SIGINT, SIGTERM, SIGHUP} )
        signal(sig, shutdownHandler);

    // Print e message if the app was not terminated graciously
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running";
    if (QFile::exists(filePath)){
        qCritical() << "Application has not been terminated graciously.";
        if (qApp->settings()->value("warnAboutNonGraciousQuit") != false){
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
                qApp->settings()->setValue("warnAboutNonGraciousQuit", false);
        }
    } else {
        // Create the running indicator file
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
            qCritical() << "Could not create file:" << filePath;
        file.close();
    }
}



/** ***************************************************************************/
AlbertApp::~AlbertApp() {

    /*
     *  FINALIZE APPLICATION
     */
    // Unload the plugins
    delete extensionManager_;
    delete hotkeyManager_;
    delete mainWindow_;

    // Delete the running indicator file
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running");
}



/** ***************************************************************************/
int AlbertApp::exec() {
    //  HOTKEY  //  Albert without hotkey is useless. Force it!
    QVariant v;
    if (!(qApp->settings()->contains("hotkey")
          && (v=qApp->settings()->value("hotkey")).canConvert(QMetaType::QString)
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
        settingsWidget_ = new SettingsWidget(mainWindow_, hotkeyManager_, extensionManager_);
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



/** ***************************************************************************/
void AlbertApp::clearInput() {
    mainWindow_->setInput("");
}



/** ***************************************************************************/
QSettings *AlbertApp::settings() {
    return settings_;
}



/** ***************************************************************************/
QString AlbertApp::term(){
    return terminal_;
}



/** ***************************************************************************/
void AlbertApp::setTerm(const QString &terminal){
    qApp->settings()->setValue(CFG_TERM, terminal);
    terminal_ = terminal;
}



/** ***************************************************************************/
void AlbertApp::enableTrayIcon(bool enable) {
    settings()->setValue(CFG_SHOWTRAY, enable);
    if (enable && trayIcon_ == nullptr)
    {
        // They cannot be parented since QMenu want QWidget as parent.
        trayIcon_ = new QSystemTrayIcon(QIcon(":app_icon"), this);
        trayIconMenu_ = new QMenu();

        QAction* quitAction     = new QAction("Quit", trayIconMenu_);
        QAction* showAction     = new QAction("Show", trayIconMenu_);
        QAction* settingsAction = new QAction("Settings", trayIconMenu_);

        quitAction->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
        showAction->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
        settingsAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));

        connect(showAction,     &QAction::triggered, this, &AlbertApp::showWidget);
        connect(settingsAction, &QAction::triggered, this, &AlbertApp::openSettings);
        connect(quitAction,     &QAction::triggered, this, &QCoreApplication::quit);

        trayIconMenu_->addAction(showAction);
        trayIconMenu_->addAction(settingsAction);
        trayIconMenu_->addSeparator();
        trayIconMenu_->addAction(quitAction);

        trayIcon_->setContextMenu(trayIconMenu_);
        trayIcon_->setVisible(true);
    }
    else if (trayIcon_ != nullptr)
    {
        trayIcon_->setVisible(false);
        trayIcon_->deleteLater();
        trayIconMenu_->deleteLater();
        trayIcon_ = nullptr;
        trayIconMenu_ = nullptr;
    }
}



/** ***************************************************************************/
bool AlbertApp::trayIconEnabled() {
    return trayIcon_ != nullptr;
}



/** ***************************************************************************/
void AlbertApp::onWidgetShown() {
    // Call all setup routines
    QFutureSynchronizer<void> synchronizer;
    for (AbstractExtension *e : extensionManager_->extensions())
        synchronizer.addFuture(QtConcurrent::run(e, &AbstractExtension::setupSession));
    synchronizer.waitForFinished();
}



/** ***************************************************************************/
void AlbertApp::onWidgetHidden() {
    // Call all teardown routines
    QFutureSynchronizer<void> synchronizer;
    for (AbstractExtension *e : extensionManager_->extensions())
        synchronizer.addFuture(QtConcurrent::run(e, &AbstractExtension::teardownSession));
    synchronizer.waitForFinished();

    // Clear the listview
    mainWindow_->setModel(nullptr);

    // Delete all the queries of this session
    for (QueryPrivate* qp : oldQueries_)
        if ( qp->isRunning() )
            connect(qp, &QueryPrivate::finished, qp, &QueryPrivate::deleteLater);
        else
            delete qp/*->deleteLater()*/;
    oldQueries_.clear();

    // TODO update the results ranking
}



/** ***************************************************************************/
void AlbertApp::onInputChanged(const QString &searchTerm) {

    if ( currentQuery_ != nullptr ) {
        // Stop last query
        disconnect(currentQuery_, &QueryPrivate::resultyReady, mainWindow_, &MainWindow::setModel);
        currentQuery_->invalidate();
        // Store old queries an delete on session teardown (listview needs the model)
        oldQueries_.push_back(currentQuery_);
    }

    // Do nothing if nothing is loaded
    if (extensionManager_->extensions().empty())
        return;

    // Start new query, if not empty
    if ( searchTerm.trimmed().isEmpty() ) {
        currentQuery_ = nullptr;
        mainWindow_->setModel(nullptr);
    } else {
        currentQuery_ = new QueryPrivate(searchTerm, extensionManager_->extensions());
        connect(currentQuery_, &QueryPrivate::resultyReady, mainWindow_, &MainWindow::setModel);
    }
}

