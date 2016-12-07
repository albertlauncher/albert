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

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <csignal>
#include "mainwindow.h"
#include "hotkeymanager.h"
#include "extensionmanager.h"
#include "queryhandler.h"
#include "settingswidget.h"
#include "trayicon.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message);
void shutdownHandler(int);
void dispatchMessage();

static const char* CFG_TERM = "terminal";
static const char* DEF_TERM = "xterm -e";

static QApplication     *app;
static MainWindow       *mainWindow;
static HotkeyManager    *hotkeyManager;
static ExtensionManager *extensionManager;
static QueryHandler     *queryHandler;
static SettingsWidget   *settingsWidget;
static TrayIcon         *trayIcon;
static QMenu            *trayIconMenu;
static QLocalServer     *localServer;
QString terminalCommand;

int main(int argc, char *argv[]) {

    {
        /*
         *  INITIALIZE APPLICATION
         */

        qInstallMessageHandler(myMessageOutput);

        app = new QApplication(argc, argv);
        app->setApplicationName("albert");
        app->setApplicationDisplayName("Albert");
        app->setApplicationVersion("v0.8.11");
        app->setWindowIcon(QIcon(":app_icon"));
        app->setQuitOnLastWindowClosed(false);


        /*
         *  PARSE COMMANDLINE
         */

        QCommandLineParser parser;
        parser.setApplicationDescription("Albert is still in alpha. These options may change in future versions.");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOption(QCommandLineOption({"k", "hotkey"}, "Overwrite the hotkey to use.", "hotkey"));
        parser.addPositionalArgument("command", "Command to send to a running instance, if any. (show, hide, toggle)", "[command]");
        parser.process(*app);


        /*
         *  SINGLE INSTANCE / IPC
         */

        const QStringList args = parser.positionalArguments();
        QLocalSocket socket;
        socket.connectToServer(app->applicationName());
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
        QLocalServer::removeServer(app->applicationName());
        localServer = new QLocalServer;
        localServer->listen(app->applicationName());
        QObject::connect(localServer, &QLocalServer::newConnection, dispatchMessage);


        /*
         *  INITIALIZE PATHS
         */

        // Make sure data, cache and config dir exists
        QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir dir;
        dir.setPath(dataLocation);
        if (!dir.mkpath("."))
            qFatal("Could not create dir: %s",  dataLocation.toUtf8().constData());
        dir.setPath(cacheLocation);
        if (!dir.mkpath("."))
            qFatal("Could not create dir: %s",  cacheLocation.toUtf8().constData());

        // Move old config for user convenience TODO drop somewhen
        QFileInfo oldcfg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/albert/albert.conf");
        QFileInfo newcfg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/albert.conf");
        if (oldcfg.exists()){
            if (newcfg.exists())
                QFile::remove(newcfg.path());
            QFile::rename(oldcfg.filePath(), newcfg.filePath());
            oldcfg.dir().removeRecursively();
        }


        /*
         * INITIALIZE DATABASE
         */

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        if (!db.driver()->hasFeature(QSqlDriver::Transactions))
            qFatal("No sqlite driver available.");
        db.setDatabaseName(QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).filePath("core.db"));
        if (!db.open())
            qFatal("Unable to establish a database connection.");

        db.transaction();

        // Creat tables
        QSqlQuery q;
        if (!q.exec("CREATE TABLE IF NOT EXISTS usages ( "
                    "  input TEXT NOT NULL, "
                    "  itemId TEXT NOT NULL, "
                    "  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP "
                    ");"))
            qFatal("Unable to create table 'usages': %s", q.lastError().text().toUtf8().constData());

        if (!q.exec("CREATE TABLE IF NOT EXISTS runtimes ( "
                    "  extensionId TEXT NOT NULL, "
                    "  runtime INTEGER NOT NULL, "
                    "  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP "
                    ");"))
            qFatal("Unable to create table 'runtimes': %s", q.lastError().text().toUtf8().constData());

        // Do regular cleanup
        if (!q.exec("DELETE FROM usages WHERE julianday('now')-julianday(timestamp)>90;"))
            qWarning("Unable to cleanup usages table.");

        if (!q.exec("DELETE FROM runtimes WHERE julianday('now')-julianday(timestamp)>7;"))
            qWarning("Unable to cleanup runtimes table.");

        db.commit();


        /*
         * INITIALIZE APPLICATION COMPONENTS
         */

        QSettings settings(qApp->applicationName());
        mainWindow       = new MainWindow;
        hotkeyManager    = new HotkeyManager;
        extensionManager = new ExtensionManager;
        queryHandler     = new QueryHandler(extensionManager);
        trayIcon         = new TrayIcon;
        settingsWidget   = new SettingsWidget(mainWindow, hotkeyManager, extensionManager, trayIcon);


        /*
         * Build Tray Icon
         */

        QAction* showAction     = new QAction("Show", trayIconMenu);
        QAction* settingsAction = new QAction("Settings", trayIconMenu);
        QAction* quitAction     = new QAction("Quit", trayIconMenu);

        showAction->setIcon(app->style()->standardIcon(QStyle::SP_TitleBarMaxButton));
        settingsAction->setIcon(app->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
        quitAction->setIcon(app->style()->standardIcon(QStyle::SP_TitleBarCloseButton));

        trayIconMenu = new QMenu;
        trayIconMenu->addAction(showAction);
        trayIconMenu->addAction(settingsAction);
        trayIconMenu->addSeparator();
        trayIconMenu->addAction(quitAction);

        trayIcon->setContextMenu(trayIconMenu);


        /*
         * SIGNALING
         */

        QObject::connect(hotkeyManager, &HotkeyManager::hotKeyPressed,
                         mainWindow, &MainWindow::toggleVisibility);

        QObject::connect(queryHandler, &QueryHandler::resultsReady,
                         mainWindow, &MainWindow::setModel);

        QObject::connect(showAction, &QAction::triggered,
                         mainWindow, &MainWindow::show);

        QObject::connect(settingsAction, &QAction::triggered,
                         settingsWidget, &SettingsWidget::show);

        QObject::connect(quitAction, &QAction::triggered,
                         app, &QApplication::quit);

        QObject::connect(trayIcon, &TrayIcon::activated,
                         mainWindow, &MainWindow::toggleVisibility);


        QObject::connect(mainWindow, &MainWindow::settingsWidgetRequested,
                         settingsWidget, &SettingsWidget::show);

        QObject::connect(mainWindow, &MainWindow::widgetShown,
                         queryHandler, &QueryHandler::setupSession);

        QObject::connect(mainWindow, &MainWindow::widgetHidden,
                         queryHandler, &QueryHandler::teardownSession);

        QObject::connect(mainWindow, &MainWindow::inputChanged,
                         queryHandler, &QueryHandler::startQuery);


        /*
         *  Hotkey
         */

        QString hotkey;
        if ( parser.isSet("hotkey") )
            hotkey = parser.value("hotkey");
        else if (settings.contains("hotkey"))
            hotkey = settings.value("hotkey", QString()).toString();
        if (!hotkey.isNull() && !hotkeyManager->registerHotkey(hotkey)) {
            QMessageBox msgBox(QMessageBox::Critical, "Error",
                               "Hotkey is not set or invalid. Do you want to open the settings?",
                               QMessageBox::No|QMessageBox::Yes);
            msgBox.exec();
            if ( msgBox.result() == QMessageBox::Yes )
                settingsWidget->show();
        }


        /*
         *  MISC
         */

        // Define the (global extern) terminal command
        terminalCommand = settings.value(CFG_TERM, DEF_TERM).toString();

        // Quit gracefully on unix signals
        for ( int sig : {SIGINT, SIGTERM, SIGHUP} )
            signal(sig, shutdownHandler);

        // Print e message if the app was not terminated graciously
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running";
        if (QFile::exists(filePath)){
            qCritical() << "Application has not been terminated graciously.";
            if (settings.value("warnAboutNonGraciousQuit") != false){
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
                    settings.setValue("warnAboutNonGraciousQuit", false);
            }
        } else {
            // Create the running indicator file
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly))
                qCritical() << "Could not create file:" << filePath;
            file.close();
        }

    }


    /*
     * ENTER EVENTLOOP
     */

    int retval = app->exec();


    /*
     *  FINALIZE APPLICATION
     */

    delete settingsWidget;
    delete trayIconMenu;
    delete trayIcon;
    delete queryHandler;
    delete extensionManager;
    delete hotkeyManager;
    delete mainWindow;

    // Delete the running indicator file
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/running");

    return retval;
}


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
void shutdownHandler(int) {
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}



/** ***************************************************************************/
void dispatchMessage() {
    QLocalSocket* socket = localServer->nextPendingConnection(); // Should be safe
    socket->waitForReadyRead(500);
    if (socket->bytesAvailable()) {
        QString msg = QString::fromLocal8Bit(socket->readAll());
        if ( msg == "show") {
            mainWindow->setVisible(true);
            socket->write("Application set visible.");
        } else if ( msg == "hide") {
            mainWindow->setVisible(false);
            socket->write("Application set invisible.");
        } else if ( msg == "toggle") {
            mainWindow->toggleVisibility();
            socket->write("Visibility toggled.");
        } else
            socket->write("Command not supported.");
    }
    socket->flush();
    socket->close();
    socket->deleteLater();
}
