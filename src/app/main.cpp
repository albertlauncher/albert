// Copyright (C) 2014-2018 Manuel Schneider

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <csignal>
#include <functional>
#include "globalshortcut/hotkeymanager.h"
#include "xdg/iconlookup.h"
#include "extensionmanager.h"
#include "albert/frontend.h"
#include "frontendmanager.h"
#include "pluginspec.h"
#include "querymanager.h"
#include "settingswidget/settingswidget.h"
#include "telemetry.h"
#include "trayicon.h"
using namespace Core;
using namespace GlobalShortcut;

static void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message);
static void dispatchMessage();
static void printReport();

// Core components
static QApplication     *app;
static ExtensionManager *extensionManager;
static FrontendManager  *frontendManager;
static QueryManager     *queryManager;
static HotkeyManager    *hotkeyManager;
static SettingsWidget   *settingsWidget;
static TrayIcon         *trayIcon;
static Telemetry        *telemetry;
static QMenu            *trayIconMenu;
static QLocalServer     *localServer;


int main(int argc, char **argv) {

    // Parse commandline
    QCommandLineParser parser;
    parser.setApplicationDescription("Albert is still in alpha. These options may change in future versions.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption({"k", "hotkey"}, "Overwrite the hotkey to use.", "hotkey"));
    parser.addOption(QCommandLineOption({"p", "plugin-dirs"}, "Set the plugin dirs to use. Comma separated.", "directory"));
    parser.addOption(QCommandLineOption({"r", "report"}, "Print issue report."));
    parser.addPositionalArgument("command", "Command to send to a running instance, if any. (show, hide, toggle)", "[command]");

    /*
     *  IPC/SINGLETON MECHANISM (Client)
     *  For performance purposes this has been optimized by using a QCoreApp
     */
    QString socketPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/socket";
    {
        QCoreApplication *capp = new QCoreApplication(argc, argv);
        capp->setApplicationName("albert");
        capp->setApplicationVersion(ALBERT_VERSION);
        parser.process(*capp);

        if (parser.isSet("report")){
            printReport();
            return EXIT_SUCCESS;
        }

        const QStringList args = parser.positionalArguments();
        QLocalSocket socket;
        socket.connectToServer(socketPath);
        if ( socket.waitForConnected(500) ) { // Should connect instantly
            // If there is a command send it
            if ( args.count() != 0 ){
                socket.write(args.join(' ').toUtf8());
                socket.flush();
                socket.waitForReadyRead(500);
                if (socket.bytesAvailable())
                    qInfo().noquote() << socket.readAll();
            }
            else
                qInfo("There is another instance of albert running.");
            socket.close();
            ::exit(EXIT_SUCCESS);
        } else if ( args.count() == 1 ) {
            qInfo("There is no other instance of albert running.");
            ::exit(EXIT_FAILURE);
        }

        delete capp;
    }


    /*
     *  INITIALIZE APPLICATION
     */
    {
        QSettings::setPath(QSettings::defaultFormat(), QSettings::UserScope,
                           QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        QSettings settings(qApp->applicationName());

        qInstallMessageHandler(myMessageOutput);

        qDebug() << "Initializing application";
#if QT_VERSION >= 0x050600  // TODO: Remove when 18.04 is released
        if (!qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO")
                && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
                && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
                && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS"))
            QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
        app = new QApplication(argc, argv);
        app->setApplicationName("albert");
        app->setApplicationDisplayName("Albert");
        app->setApplicationVersion(ALBERT_VERSION);
        app->setQuitOnLastWindowClosed(false);
        QString icon = XDG::IconLookup::iconPath("albert");
        if ( icon.isEmpty() ) icon = ":app_icon";
        app->setWindowIcon(QIcon(icon));



        /*
         *  IPC/SINGLETON MECHANISM (Server)
         */

        // Remove pipes potentially leftover after crash
        QLocalServer::removeServer(socketPath);

        // Create server and handle messages
        qDebug() << "Creating IPC server";
        localServer = new QLocalServer;
        if ( !localServer->listen(socketPath) )
            qWarning() << "Local server could not be created. IPC will not work! Reason:"
                       << localServer->errorString();

        // Handle incoming messages
        QObject::connect(localServer, &QLocalServer::newConnection, dispatchMessage);



        /*
         *  INITIALIZE PATHS
         */

        // Make sure data, cache and config dir exists
        qDebug() << "Initializing mandatory paths";
        QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QString configLocation = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        for ( const QString &location : {dataLocation, cacheLocation, configLocation} )
            if (!QDir(location).mkpath("."))
                qFatal("Could not create dir: %s",  qPrintable(location));



        /*
         *  ADJUST PATHS OF FILES OF OLDER VERSIONS
         */

        // If there is a firstRun file, rename it to lastVersion (since v0.11)
        if ( QFile::exists(QString("%1/firstrun").arg(dataLocation)) ) {
            qDebug() << "Renaming 'firstrun' to 'last_used_version'";
            QFile::rename(QString("%1/firstrun").arg(dataLocation),
                          QString("%1/last_used_version").arg(dataLocation));
        }

        // Move old config for user convenience  (since v0.13)
        QFileInfo oldcfg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/albert.conf");
        QFileInfo newcfg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/albert/albert.conf");
        if (oldcfg.exists()){
            if (newcfg.exists())
                QFile::remove(newcfg.filePath());
            QFile::rename(oldcfg.filePath(), newcfg.filePath());
        }

        // If there is a lastVersion  file move it to config (since v0.13)
        if ( QFile::exists(QString("%1/last_used_version").arg(dataLocation)) ) {
            qDebug() << "Moving 'last_used_version' to config path";
            QFile::rename(QString("%1/last_used_version").arg(dataLocation),
                          QString("%1/last_used_version").arg(configLocation));
        }

        // If move database from old location in cache to config (since v0.14.7)
        if ( QFile::exists(QString("%1/core.db").arg(cacheLocation)) ){
            qInfo() << "Moving 'core.db' to config path";
            QFile::rename(QString("%1/core.db").arg(cacheLocation),
                          QString("%1/core.db").arg(configLocation));
        }


        /*
         *  MISC
         */

        // Quit gracefully on unix signals
        qDebug() << "Setup signal handlers";
        for ( int sig : { SIGINT, SIGTERM, SIGHUP, SIGPIPE } ) {
            signal(sig, [](int){
                QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
            });
        }

        // Print a message if the app was not terminated graciously
        qDebug() << "Creating running indicator file";
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running";
        if (QFile::exists(filePath)){
            qWarning() << "Application has not been terminated graciously.";
        } else {
            // Create the running indicator file
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly))
                qWarning() << "Could not create file:" << filePath;
            file.close();
        }



        /*
         *  INITIALIZE CORE DATABASE
         */


        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        if ( !db.isValid() )
            qFatal("No sqlite available");
        if (!db.driver()->hasFeature(QSqlDriver::Transactions))
            qFatal("QSqlDriver::Transactions not available.");
        db.setDatabaseName(QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).filePath("core.db"));
        if (!db.open())
            qFatal("Unable to establish a database connection.");

        db.transaction();

        QSqlQuery q(db);
        if (!q.exec("CREATE TABLE IF NOT EXISTS query_handler ( "
                    "  id INTEGER PRIMARY KEY NOT NULL, "
                    "  string_id TEXT UNIQUE NOT NULL "
                    "); "))
            qFatal("Unable to create table 'query_handler': %s", q.lastError().text().toUtf8().constData());

        if (!q.exec("CREATE TABLE IF NOT EXISTS query ( "
                    "    id INTEGER PRIMARY KEY, "
                    "    input TEXT NOT NULL, "
                    "    cancelled INTEGER NOT NULL, "
                    "    runtime INTEGER NOT NULL, "
                    "    timestamp INTEGER DEFAULT CURRENT_TIMESTAMP "
                    "); "))
            qFatal("Unable to create table 'query': %s", q.lastError().text().toUtf8().constData());

        if (!q.exec("CREATE TABLE IF NOT EXISTS execution ( "
                    "    query_id INTEGER NOT NULL REFERENCES query(id) ON UPDATE CASCADE, "
                    "    handler_id INTEGER NOT NULL REFERENCES query_handler(id) ON UPDATE CASCADE, "
                    "    runtime INTEGER NOT NULL, "
                    "    PRIMARY KEY (query_id, handler_id) "
                    ") WITHOUT ROWID; "))
            qFatal("Unable to create table 'execution': %s", q.lastError().text().toUtf8().constData());

        if (!q.exec("CREATE TABLE IF NOT EXISTS activation ( "
                    "    query_id INTEGER PRIMARY KEY NOT NULL REFERENCES query(id) ON UPDATE CASCADE, "
                    "    item_id TEXT NOT NULL "
                    "); "))
            qFatal("Unable to create table 'activation': %s", q.lastError().text().toUtf8().constData());

        if (!q.exec("DELETE FROM query WHERE julianday('now')-julianday(timestamp)>30; "))
            qWarning("Unable to cleanup 'query' table.");

        if (!q.exec("CREATE TABLE IF NOT EXISTS conf(key TEXT UNIQUE, value TEXT); "))
            qFatal("Unable to create table 'conf': %s", q.lastError().text().toUtf8().constData());

        db.commit();


        /*
         *  INITIALIZE APPLICATION COMPONENTS
         */

        qDebug() << "Initializing core components";

        // Define plugindirs
        QStringList pluginDirs;
        if ( parser.isSet("plugin-dirs") )
            pluginDirs = parser.value("plugin-dirs").split(',');
        else {
#if defined __linux__
            QStringList dirs = {
#if defined MULTIARCH_TUPLE
                QFileInfo("/usr/lib/" MULTIARCH_TUPLE).canonicalFilePath(),
#endif
                QFileInfo("/usr/lib/").canonicalFilePath(),
                QFileInfo("/usr/lib64/").canonicalFilePath(),
                QFileInfo("/usr/local/lib/").canonicalFilePath(),
                QFileInfo("/usr/local/lib64/").canonicalFilePath(),
                QDir::home().filePath(".local/lib/"),
                QDir::home().filePath(".local/lib64/")
            };

            dirs.removeDuplicates();

            for ( const QString& dir : dirs ) {
                QFileInfo fileInfo = QFileInfo(QDir(dir).filePath("albert/plugins"));
                if ( fileInfo.isDir() )
                    pluginDirs.push_back(fileInfo.canonicalFilePath());
            }
#elif defined __APPLE__
        throw "Not implemented";
#elif defined _WIN32
        throw "Not implemented";
#endif
        }

        frontendManager = new FrontendManager(pluginDirs);
        extensionManager = new ExtensionManager(pluginDirs);
        extensionManager->reloadExtensions();
        hotkeyManager = new HotkeyManager;
        if ( parser.isSet("hotkey") ) {
            QString hotkey = parser.value("hotkey");
            if ( !hotkeyManager->registerHotkey(hotkey) )
                qFatal("Failed to set hotkey to %s.", hotkey.toLocal8Bit().constData());
        } else if ( settings.contains("hotkey") ) {
            QString hotkey = settings.value("hotkey").toString();
            if ( !hotkeyManager->registerHotkey(hotkey) )
                qFatal("Failed to set hotkey to %s.", hotkey.toLocal8Bit().constData());
        }
        queryManager = new QueryManager(extensionManager);
        telemetry  = new Telemetry;
        trayIcon = new TrayIcon;
        trayIconMenu  = new QMenu;
        settingsWidget = new SettingsWidget(extensionManager,
                                            frontendManager,
                                            queryManager,
                                            hotkeyManager,
                                            trayIcon,
                                            telemetry);

        QAction* showAction     = new QAction("Show", trayIconMenu);
        showAction->setIcon(app->style()->standardIcon(QStyle::SP_TitleBarMaxButton));
        trayIconMenu->addAction(showAction);

        QAction* settingsAction = new QAction("Settings", trayIconMenu);
        settingsAction->setIcon(app->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
        trayIconMenu->addAction(settingsAction);
        QObject::connect(settingsAction, &QAction::triggered, [](){
            settingsWidget->show();
            settingsWidget->raise();
        });

        QAction* docsAction = new QAction("Open docs", trayIconMenu);
        docsAction->setIcon(app->style()->standardIcon(QStyle::SP_DialogHelpButton));
        trayIconMenu->addAction(docsAction);
        QObject::connect(docsAction, &QAction::triggered, [](){
            QDesktopServices::openUrl(QUrl("https://albertlauncher.github.io/docs/"));
        });

        trayIconMenu->addSeparator();
        QAction* quitAction = new QAction("Quit", trayIconMenu);
        quitAction->setIcon(app->style()->standardIcon(QStyle::SP_TitleBarCloseButton));
        trayIconMenu->addAction(quitAction);
        QObject::connect(quitAction, &QAction::triggered, app, &QApplication::quit);

        trayIcon->setContextMenu(trayIconMenu);



        /*
         *  DETECT FIRST RUN AND VERSION CHANGE
         */

        qDebug() << "Checking last used version";
        QFile file(QString("%1/last_used_version").arg(configLocation));
        if ( file.exists() ) {
            // Read last used version
            if ( file.open(QIODevice::ReadOnly|QIODevice::Text) ) {
                QString lastUsedVersion;
                QTextStream(&file) >> lastUsedVersion;
                file.close();

                // Show newsbox in case of major version change
                if ( app->applicationVersion().section('.', 1, 1) != lastUsedVersion.section('.', 1, 1) ){
                    // Do whatever is neccessary on first run
                    QMessageBox(QMessageBox::Information, "Major version changed",
                                QString("You are now using Albert %1. Albert is still in the alpha "
                                        "stage. This means things may change unexpectedly. Check "
                                        "the <a href=\"https://albertlauncher.github.io/news/\">"
                                        "news</a> to read about the things that changed.")
                                .arg(app->applicationVersion())).exec();
                }
            }
            else
                qCritical() << qPrintable(QString("Could not open file %1: %2,. Config migration may fail.")
                                          .arg(file.fileName(), file.errorString()));
        } else {
            // Do whatever is neccessary on first run
            QMessageBox(QMessageBox::Information, "First run",
                        "Seems like this is the first time you run Albert. Albert is "
                        "standalone, free and open source software. Note that Albert is not "
                        "related to or affiliated with any other projects or corporations.\n\n"
                        "You should set a hotkey and enable some extensions.").exec();
            settingsWidget->show();
        }

        // Write the current version into the file
        if ( file.open(QIODevice::WriteOnly|QIODevice::Text) ) {
            QTextStream out(&file);
            out << app->applicationVersion();
            file.close();
        } else
            qCritical() << qPrintable(QString("Could not open file %1: %2").arg(file.fileName(), file.errorString()));



        /*
         * SIGNALING
         */

        // Define a lambda that connects a new frontend
        auto connectFrontend = [&](Frontend *f){

            QObject::connect(hotkeyManager, &HotkeyManager::hotKeyPressed,
                             f, &Frontend::toggleVisibility);

            QObject::connect(queryManager, &QueryManager::resultsReady,
                             f, &Frontend::setModel);

            QObject::connect(showAction, &QAction::triggered,
                             f, &Frontend::setVisible);

            QObject::connect(trayIcon, &TrayIcon::activated,
                             f, [=](QSystemTrayIcon::ActivationReason reason){
                if( reason == QSystemTrayIcon::ActivationReason::Trigger)
                    f->toggleVisibility();
            });

            QObject::connect(f, &Frontend::settingsWidgetRequested, [](){
                settingsWidget->show();
                settingsWidget->raise();
                settingsWidget->activateWindow();
            });

            QObject::connect(f, &Frontend::widgetShown, [f](){
                queryManager->setupSession();
                queryManager->startQuery(f->input());
            });

            QObject::connect(f, &Frontend::widgetHidden,
                             queryManager, &QueryManager::teardownSession);

            QObject::connect(f, &Frontend::inputChanged,
                             queryManager, &QueryManager::startQuery);
        };

        // Connect the current frontend
        connectFrontend(frontendManager->currentFrontend());

        // Connect new frontends
        QObject::connect(frontendManager, &FrontendManager::frontendChanged, connectFrontend);
    }


    /*
     * ENTER EVENTLOOP
     */

    qDebug() << "Entering eventloop";
    int retval = app->exec();


    /*
     *  FINALIZE APPLICATION
     */

    qDebug() << "Cleaning up core components";
    delete settingsWidget;
    delete trayIconMenu;
    delete trayIcon;
    delete queryManager;
    delete hotkeyManager;
    delete extensionManager;
    delete frontendManager;

    qDebug() << "Shutting down IPC server";
    localServer->close();

    // Delete the running indicator file
    qDebug() << "Deleting running indicator file";
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running");

    qDebug() << "Quit";
    return retval;
}


/** ***************************************************************************/
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message) {
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "%s \x1b[34;1m[DEBG:%s]\x1b[0m \x1b[3m%s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtInfoMsg:
        fprintf(stdout, "%s \x1b[32;1m[INFO:%s]\x1b[0m %s\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtWarningMsg:
        fprintf(stdout, "%s \x1b[33;1m[WARN:%s]\x1b[0;1m %s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtCriticalMsg:
        fprintf(stdout, "%s \x1b[31;1m[CRIT:%s]\x1b[0;1m %s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s \x1b[41;30;4m[FATAL:%s]\x1b[0;1m %s  --  [%s]\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData(),
                context.function);
        exit(1);
    }
    fflush(stdout);
}


/** ***************************************************************************/
void dispatchMessage() {
    QLocalSocket* socket = localServer->nextPendingConnection(); // Should be safe
    socket->waitForReadyRead(500);
    if (socket->bytesAvailable()) {
        QString msg = QString::fromLocal8Bit(socket->readAll());
        if ( msg.startsWith("show")) {
            if (msg.size() > 5) {
                QString input = msg.mid(5);
                frontendManager->currentFrontend()->setInput(input);
            }
            frontendManager->currentFrontend()->setVisible(true);
            socket->write("Application set visible.");
        } else if ( msg == "hide") {
            frontendManager->currentFrontend()->setVisible(false);
            socket->write("Application set invisible.");
        } else if ( msg == "toggle") {
            frontendManager->currentFrontend()->toggleVisibility();
            socket->write("Visibility toggled.");
        } else
            socket->write("Command not supported.");
    }
    socket->flush();
    socket->close();
    socket->deleteLater();
}


/** ***************************************************************************/
static void printReport()
{
    const uint8_t w = 22;
    qInfo().noquote() << QString("%1: %2").arg("Albert version", w).arg(qApp->applicationVersion());
    qInfo().noquote() << QString("%1: %2").arg("Build date", w).arg(__DATE__ " " __TIME__);

    qInfo().noquote() << QString("%1: %2").arg("Qt version", w).arg(qVersion());
    qInfo().noquote() << QString("%1: %2").arg("QT_QPA_PLATFORMTHEME", w).arg(QString::fromLocal8Bit(qgetenv("QT_QPA_PLATFORMTHEME")));

    qInfo().noquote() << QString("%1: %2").arg("Binary location", w).arg(qApp->applicationFilePath());

    qInfo().noquote() << QString("%1: %2").arg("PWD", w).arg(QString::fromLocal8Bit(qgetenv("PWD")));
    qInfo().noquote() << QString("%1: %2").arg("SHELL", w).arg(QString::fromLocal8Bit(qgetenv("SHELL")));
    qInfo().noquote() << QString("%1: %2").arg("LANG", w).arg(QString::fromLocal8Bit(qgetenv("LANG")));

    qInfo().noquote() << QString("%1: %2").arg("XDG_SESSION_TYPE", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE")));
    qInfo().noquote() << QString("%1: %2").arg("XDG_CURRENT_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")));
    qInfo().noquote() << QString("%1: %2").arg("DESKTOP_SESSION", w).arg(QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION")));
    qInfo().noquote() << QString("%1: %2").arg("XDG_SESSION_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_DESKTOP")));

    qInfo().noquote() << QString("%1: %2").arg("OS", w).arg(QSysInfo::prettyProductName());
    qInfo().noquote() << QString("%1: %2/%3").arg("OS (type/version)", w).arg(QSysInfo::productType(), QSysInfo::productVersion());

    qInfo().noquote() << QString("%1: %2").arg("Build ABI", w).arg(QSysInfo::buildAbi());
    qInfo().noquote() << QString("%1: %2/%3").arg("Arch (build/current)", w).arg(QSysInfo::buildCpuArchitecture(), QSysInfo::currentCpuArchitecture());

    qInfo().noquote() << QString("%1: %2/%3").arg("Kernel (type/version)", w).arg(QSysInfo::kernelType(), QSysInfo::kernelVersion());
}
