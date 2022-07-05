// Copyright (C) 2014-2018 Manuel Schneider

#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QLoggingCategory>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
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
#if defined __linux__ || defined __freebsd__
#include <QX11Info>
#include "xcb/xproto.h"
#endif
#include <csignal>
#include <functional>
#include "albert/frontend.h"
#include "albert/queryhandler.h"
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "extensionmanager.h"
#include "frontendmanager.h"
#include "globalshortcut/hotkeymanager.h"
#include "logging.h"
#include "pluginspec.h"
#include "querymanager.h"
#include "settingswidget/settingswidget.h"
#include "trayicon.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(clc, "core")
using namespace Core;
using namespace std;
using namespace GlobalShortcut;

static void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message);
static void printReport();
struct CoreQueryHandler : public Core::QueryHandler
{
    CoreQueryHandler(const vector<shared_ptr<Item>>& items)
        : Core::QueryHandler("org.albert"), items(items){}

    void handleQuery(Core::Query * query) const override {
        for (auto item : items)
            if (!query->string().isEmpty() && item->text().toLower().startsWith(query->string().toLower()))
                  query->addMatch(item, static_cast<uint>((query->string().length() / item->text().length()) * numeric_limits<uint>::max()));
    }
    vector<shared_ptr<Item>> items;
};
struct GlobalNativeEventFilter : public QAbstractNativeEventFilter {
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override;
};


int main(int argc, char **argv) {

    // Parse commandline
    QCommandLineParser parser;
    parser.setApplicationDescription("Albert is still in alpha. These options may change in future versions.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption({"k", "hotkey"}, "Overwrite the hotkey to use.", "hotkey"));
    parser.addOption(QCommandLineOption({"p", "plugin-dirs"}, "Set the plugin dirs to use. Comma separated.", "directory"));
    parser.addOption(QCommandLineOption({"r", "report"}, "Print issue report."));
    parser.addPositionalArgument("command", "Command to send to a running instance, if any. (show, hide, toggle, preferences, restart, quit)", "lbert -h");

    /*
     *  IPC/SINGLETON MECHANISM (Client)
     *  For performance purposes this has been optimized by using a QCoreApp
     */
    QString socketPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/socket";
#if X_PROTOCOL
    socketPath.append(QString::number(QX11Info::appScreen()));
#endif
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
                    INFO << socket.readAll();
            }
            else
                INFO << "There is another instance of albert running.";
            socket.close();
            ::exit(EXIT_SUCCESS);
        } else if ( args.count() == 1 ) {
            INFO << "There is no other instance of albert running.";
            ::exit(EXIT_FAILURE);
        }

        delete capp;
    }


    /*
     *  INITIALIZE APPLICATION
     */

    unique_ptr<GlobalNativeEventFilter> gnev = make_unique<GlobalNativeEventFilter>();
    unique_ptr<QApplication> app;
    unique_ptr<FrontendManager> frontendManager;
    unique_ptr<ExtensionManager> extensionManager;
    unique_ptr<HotkeyManager> hotkeyManager;
    unique_ptr<QueryManager> queryManager;
    unique_ptr<SettingsWidget> settingsWidget;

    unique_ptr<CoreQueryHandler> coreQueryHandler;
    unique_ptr<TrayIcon> trayIcon;
    unique_ptr<QMenu> trayIconMenu;
    unique_ptr<QLocalServer> localServer;

    {
        QSettings::setPath(QSettings::defaultFormat(), QSettings::UserScope,
                           QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        QSettings settings(qApp->applicationName());

        qInstallMessageHandler(myMessageOutput);

        INFO << "Initializing application";
#if QT_VERSION >= 0x050600  // TODO: Remove when 18.04 is released
        if (!qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO")
                && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
                && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
                && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS"))
            QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
        app = make_unique<QApplication>(argc, argv);
        app->setApplicationName("albert");
        app->setApplicationDisplayName("Albert");
        app->setApplicationVersion(ALBERT_VERSION);
        app->setQuitOnLastWindowClosed(false);
        QString icon = XDG::IconLookup::iconPath("albert");
        if ( icon.isEmpty() ) icon = ":app_icon";
        app->setWindowIcon(QIcon(icon));
        app->installNativeEventFilter(gnev.get());


        /*
         *  INITIALIZE PATHS
         */

        // Make sure data, cache and config dir exists
        INFO << "Initializing mandatory paths";
        QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QString configLocation = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        for ( const QString &location : {dataLocation, cacheLocation, configLocation} )
            if (!QDir(location).mkpath("."))
                qFatal("Could not create dir: %s",  qPrintable(location));


        /*
         *  MISC
         */

        // Quit gracefully on unix signals
        INFO << "Setup signal handlers";
        for ( int sig : { SIGINT, SIGTERM, SIGHUP, SIGPIPE } ) {
            signal(sig, [](int){
                QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
            });
        }

        // Print a message if the app was not terminated graciously
        INFO << "Creating running indicator file";
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running";
        if (QFile::exists(filePath)){
            WARN << "Application has not been terminated graciously.";
        } else {
            // Create the running indicator file
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly))
                WARN << "Could not create file:" << filePath;
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

        if (!q.exec("CREATE TABLE IF NOT EXISTS conf(key TEXT UNIQUE, value TEXT); "))
            qFatal("Unable to create table 'conf': %s", q.lastError().text().toUtf8().constData());

        db.commit();


        /*
         *  INITIALIZE APPLICATION COMPONENTS
         */

        INFO << "Initializing core components";

        // Define plugindirs
        QStringList pluginDirs;
        if ( parser.isSet("plugin-dirs") )
            pluginDirs = parser.value("plugin-dirs").split(',');
        else {
#if defined __linux__ || defined __FreeBSD__
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

        frontendManager = make_unique<FrontendManager>(pluginDirs);
        extensionManager = make_unique<ExtensionManager>(pluginDirs);
        extensionManager->reloadExtensions();

        if ( !QGuiApplication::platformName().contains("wayland") )
            hotkeyManager = make_unique<HotkeyManager>();

        if ( hotkeyManager ) {
            if ( parser.isSet("hotkey") ) {
                QString hotkey = parser.value("hotkey");
                if ( !hotkeyManager->registerHotkey(hotkey) )
                    qFatal("Failed to set hotkey to %s.", hotkey.toLocal8Bit().constData());
            } else if ( settings.contains("hotkey") ) {
                QString hotkey = settings.value("hotkey").toString();
                if ( !hotkeyManager->registerHotkey(hotkey) )
                    qFatal("Failed to set hotkey to %s.", hotkey.toLocal8Bit().constData());
            }
        }
        queryManager = make_unique<QueryManager>(extensionManager.get());
        trayIcon = make_unique<TrayIcon>();
        trayIconMenu  = make_unique<QMenu>();
        settingsWidget = make_unique<SettingsWidget>(extensionManager.get(),
                                                     frontendManager.get(),
                                                     queryManager.get(),
                                                     hotkeyManager.get(),
                                                     trayIcon.get());


        QAction* showAction = new QAction("Show", trayIconMenu.get());
        trayIconMenu->addAction(showAction);

        QAction* settingsAction = new QAction("Settings", trayIconMenu.get());
        QObject::connect(settingsAction, &QAction::triggered, [&](){
            settingsWidget->show();
            settingsWidget->raise();
        });
        trayIconMenu->addAction(settingsAction);

        QAction* docsAction = new QAction("Open docs", trayIconMenu.get());
        QObject::connect(docsAction, &QAction::triggered, [](){
            QDesktopServices::openUrl(QUrl("https://albertlauncher.github.io/"));
        });
        trayIconMenu->addAction(docsAction);

        trayIconMenu->addSeparator();

        QAction* restartAction = new QAction("Restart", trayIconMenu.get());
        QObject::connect(restartAction, &QAction::triggered, [](){
            QStringList cmdline(qApp->arguments());
            if (QProcess::startDetached(cmdline.takeFirst(), cmdline)){
                // TODO: Potential race conditions on slow systems
                INFO << "Restarting application:" << cmdline;
                qApp->quit();
            } else {
                WARN << "Restarting application failed:" << cmdline;
            }
        });
        trayIconMenu->addAction(restartAction);

        QAction* quitAction = new QAction("Quit", trayIconMenu.get());
        QObject::connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
        trayIconMenu->addAction(quitAction);

        trayIcon->setContextMenu(trayIconMenu.get());



        /*
         *  DETECT FIRST RUN AND VERSION CHANGE
         */

        INFO << "Checking last used version";
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
                                QString("You are now using Albert %1. The major version changed. "
                                        "Probably some parts of the API changed. Check the "
                                        "<a href=\"https://albertlauncher.github.io/news/\">news</a>.")
                                .arg(app->applicationVersion())).exec();
                }
            }
            else
                CRIT << QString("Could not open file %1: %2")
                        .arg(file.fileName(), file.errorString());
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
            CRIT << QString("Could not open file %1: %2").arg(file.fileName(), file.errorString());


        /*
         *  IPC/SINGLETON MECHANISM (Server)
         */

        // Remove pipes potentially leftover after crash
        QLocalServer::removeServer(socketPath);

        // Create server and handle messages
        INFO << "Creating IPC server";
        localServer = make_unique<QLocalServer>();
        if ( !localServer->listen(socketPath) )
            WARN << "Local server could not be created. IPC will not work! Reason:"
                       << localServer->errorString();

        // Handle incoming messages
        QObject::connect(localServer.get(), &QLocalServer::newConnection, [&](){
            QLocalSocket* socket = localServer->nextPendingConnection();
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
                }
                else if ( msg == "hide") {
                    frontendManager->currentFrontend()->setVisible(false);
                    socket->write("Application set invisible.");
                }
                else if ( msg == "toggle") {
                    frontendManager->currentFrontend()->toggleVisibility();
                    socket->write("Visibility toggled.");
                }
                else if ( msg == "preferences") {
                    settingsAction->trigger();
                    socket->write("Preferences opened.");
                }
                else if ( msg == "restart") {
                    quitAction->trigger();
                    socket->write("Albert restart triggered.");
                }
                else if ( msg == "quit") {
                    quitAction->trigger();
                    socket->write("Albert shutdown triggered.");
                }
                else
                    socket->write("Command not supported.");
            }
            socket->flush();
            socket->close();
            socket->deleteLater();

        });

        // Core items
        coreQueryHandler = make_unique<CoreQueryHandler>(initializer_list<shared_ptr<Item>>{
            makeStdItem("open-preferences",
                ":app_icon", "Preferences", "Open the Albert preferences window.",
                initializer_list<shared_ptr<Action>>{
                    makeFuncAction("Open preferences.", [=](){ settingsAction->trigger(); })
                }
            ),
            makeStdItem("restart-albert",
                ":app_icon", "Restart Albert", "Restart this application.",
                initializer_list<shared_ptr<Action>>{
                    makeFuncAction("Restart Albert", [=](){ restartAction->trigger(); })
                }
            ),
            makeStdItem("quit-albert",
                ":app_icon", "Quit Albert", "Quit this application.",
                initializer_list<shared_ptr<Action>>{
                    makeFuncAction("Quit Albert", [=](){ quitAction->trigger(); })
                }
            )
        });

        extensionManager->registerQueryHandler(coreQueryHandler.get());

        /*
         * SIGNALING
         */

        // Define a lambda that connects a new frontend
        auto connectFrontend = [&](Frontend *f){

            QObject::connect(hotkeyManager.get(), &HotkeyManager::hotKeyPressed,
                             f, &Frontend::toggleVisibility);

            QObject::connect(queryManager.get(), &QueryManager::resultsReady,
                             f, &Frontend::setModel);

            QObject::connect(showAction, &QAction::triggered,
                             f, &Frontend::setVisible);

            QObject::connect(trayIcon.get(), &TrayIcon::activated,
                             f, [=](QSystemTrayIcon::ActivationReason reason){
                if( reason == QSystemTrayIcon::ActivationReason::Trigger)
                    f->toggleVisibility();
            });

            QObject::connect(f, &Frontend::settingsWidgetRequested, [&settingsWidget](){
                settingsWidget->show();
                settingsWidget->raise();
                settingsWidget->activateWindow();
            });

            QObject::connect(f, &Frontend::widgetShown, [f, &queryManager](){
                queryManager->setupSession();
                queryManager->startQuery(f->input());
            });

            QObject::connect(f, &Frontend::widgetHidden,
                             queryManager.get(), &QueryManager::teardownSession);

            QObject::connect(f, &Frontend::inputChanged,
                             queryManager.get(), &QueryManager::startQuery);
        };

        // Connect the current frontend
        connectFrontend(frontendManager->currentFrontend());

        // Connect new frontends
        QObject::connect(frontendManager.get(), &FrontendManager::frontendChanged, connectFrontend);
    }

    /*
     * ENTER EVENTLOOP
     */

    INFO << "Entering eventloop";
    int retval = app->exec();


    /*
     *  FINALIZE APPLICATION
     */
    INFO << "Shutting down IPC server";
    localServer->close();

    // Delete the running indicator file
    INFO << "Deleting running indicator file";
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running");

    INFO << "Quit";
    return retval;
}


/** ***************************************************************************/
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message) {
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "%s \x1b[34;1m[debg:%s]\x1b[0m \x1b[3m%s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtInfoMsg:
        fprintf(stdout, "%s \x1b[32;1m[info:%s]\x1b[0m %s\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtWarningMsg:
        fprintf(stdout, "%s \x1b[33;1m[warn:%s]\x1b[0;1m %s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtCriticalMsg:
        fprintf(stdout, "%s \x1b[31;1m[crit:%s]\x1b[0;1m %s\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s \x1b[41;30;4m[fatal:%s]\x1b[0;1m %s  --  [%s]\x1b[0m\n",
                QTime::currentTime().toString().toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData(),
                context.function);
        exit(1);
    }
    fflush(stdout);
}

/** ***************************************************************************/
static void printReport()
{
    const uint8_t w = 22;
    INFO << QString("%1: %2").arg("Albert version", w).arg(qApp->applicationVersion());
    INFO << QString("%1: %2").arg("Build date", w).arg(__DATE__ " " __TIME__);

    INFO << QString("%1: %2").arg("Qt version", w).arg(qVersion());
    INFO << QString("%1: %2").arg("QT_QPA_PLATFORMTHEME", w).arg(QString::fromLocal8Bit(qgetenv("QT_QPA_PLATFORMTHEME")));

    INFO << QString("%1: %2").arg("Binary location", w).arg(qApp->applicationFilePath());

    INFO << QString("%1: %2").arg("PWD", w).arg(QString::fromLocal8Bit(qgetenv("PWD")));
    INFO << QString("%1: %2").arg("SHELL", w).arg(QString::fromLocal8Bit(qgetenv("SHELL")));
    INFO << QString("%1: %2").arg("LANG", w).arg(QString::fromLocal8Bit(qgetenv("LANG")));

    INFO << QString("%1: %2").arg("XDG_SESSION_TYPE", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE")));
    INFO << QString("%1: %2").arg("XDG_CURRENT_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")));
    INFO << QString("%1: %2").arg("DESKTOP_SESSION", w).arg(QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION")));
    INFO << QString("%1: %2").arg("XDG_SESSION_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_DESKTOP")));

    INFO << QString("%1: %2").arg("OS", w).arg(QSysInfo::prettyProductName());
    INFO << QString("%1: %2/%3").arg("OS (type/version)", w).arg(QSysInfo::productType(), QSysInfo::productVersion());

    INFO << QString("%1: %2").arg("Build ABI", w).arg(QSysInfo::buildAbi());
    INFO << QString("%1: %2/%3").arg("Arch (build/current)", w).arg(QSysInfo::buildCpuArchitecture(), QSysInfo::currentCpuArchitecture());

    INFO << QString("%1: %2/%3").arg("Kernel (type/version)", w).arg(QSysInfo::kernelType(), QSysInfo::kernelVersion());
}


bool GlobalNativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *){
#if defined __linux__ || defined __FreeBSD__
    if (eventType == "xcb_generic_event_t")
    {
        /* This is a horribly hackish but working solution.

         A triggered key grab on X11 steals the focus of the window for short
         period of time. This may result in the following annoying behaviour:
         When the hotkey is pressed and X11 steals the focus there arises a
         race condition between the hotkey event and the focus out event.
         When the app is visible and the focus out event is delivered the app
         gets hidden. Finally when the hotkey is received the app gets shown
         again although the user intended to hide the app with the hotkey.

         Solutions:
         Although X11 differs between the two focus out events, qt does not.
         One might install a native event filter and use the XCB structs to
         decide which type of event is delivered, but this approach is not
         platform independent (unless designed so explicitely, but its a
         hassle). The behaviour was expected when the app hides on:

         (mode==XCB_NOTIFY_MODE_GRAB && detail==XCB_NOTIFY_DETAIL_NONLINEAR)||
          (mode==XCB_NOTIFY_MODE_NORMAL && detail==XCB_NOTIFY_DETAIL_NONLINEAR)
         (Check Xlib Programming Manual)

         Another much simpler but less elegant solution is to delay the
         hiding a few milliseconds, so that the hotkey event will always be
         handled first. */

        xcb_generic_event_t* event = static_cast<xcb_generic_event_t *>(message);
        switch (event->response_type & 127)
        {
        case XCB_FOCUS_OUT: {
            xcb_focus_out_event_t *fe = reinterpret_cast<xcb_focus_out_event_t*>(event);
            std::string msg = "XCB_FOCUS_OUT";

            switch (fe->mode) {
                case XCB_NOTIFY_MODE_NORMAL:        msg += "::XCB_NOTIFY_MODE_NORMAL";break;
                case XCB_NOTIFY_MODE_GRAB:          msg += "::XCB_NOTIFY_MODE_GRAB";break;
                case XCB_NOTIFY_MODE_UNGRAB:        msg += "::XCB_NOTIFY_MODE_UNGRAB";break;
                case XCB_NOTIFY_MODE_WHILE_GRABBED: msg += "::XCB_NOTIFY_MODE_WHILE_GRABBED";break;
            }
            switch (fe->detail) {
                case XCB_NOTIFY_DETAIL_ANCESTOR:          msg += "::ANCESTOR";break;
                case XCB_NOTIFY_DETAIL_INFERIOR:          msg += "::INFERIOR";break;
                case XCB_NOTIFY_DETAIL_NONE:              msg += "::NONE";break;
                case XCB_NOTIFY_DETAIL_NONLINEAR:         msg += "::NONLINEAR";break;
                case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL: msg += "::NONLINEAR_VIRTUAL";break;
                case XCB_NOTIFY_DETAIL_POINTER:           msg += "::POINTER";break;
                case XCB_NOTIFY_DETAIL_POINTER_ROOT:      msg += "::POINTER_ROOT";break;
                case XCB_NOTIFY_DETAIL_VIRTUAL:           msg += "::VIRTUAL";break;
            }
            if (fe->mode==XCB_NOTIFY_MODE_NORMAL && fe->detail==XCB_NOTIFY_DETAIL_NONLINEAR )
                return false;
            else
                return true;  // Stop propagation

        }
        }
    }
#endif
    return false;
}
