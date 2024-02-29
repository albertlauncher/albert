// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/config.h"
#include "albert/extension/frontend/frontend.h"
#include "albert/logging.h"
#include "albert/util/iconprovider.h"
#include "app.h"
#include "report.h"
#include <QApplication>
#include <QClipboard>
#include <QCommandLineParser>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTranslator>
#include <QLibraryInfo>
#if __has_include(<unistd.h>)
#include "platform/Unix/unixsignalhandler.h"
#endif
Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert")
using namespace std;
using namespace albert;
static App *app;


static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    // Todo use std::format as soon as apple gets it off the ground
    switch (type) {
        case QtDebugMsg:
            fprintf(stdout, "%s \x1b[34m[debg:%s]\x1b[0m %s\x1b[0m\n",
                    QTime::currentTime().toString().toLocal8Bit().constData(),
                    context.category,
                    message.toLocal8Bit().constData());
            break;
        case QtInfoMsg:
            fprintf(stdout, "%s \x1b[32m[info:%s]\x1b[0m %s\n",
                    QTime::currentTime().toString().toLocal8Bit().constData(),
                    context.category,
                    message.toLocal8Bit().constData());

            break;
        case QtWarningMsg:
            fprintf(stdout, "%s \x1b[33m[warn:%s]\x1b[0m %s\x1b[0m\n",
                    QTime::currentTime().toString().toLocal8Bit().constData(),
                    context.category,
                    message.toLocal8Bit().constData());
            break;
        case QtCriticalMsg:
            fprintf(stdout, "%s \x1b[31m[crit:%s] %s\x1b[0m\n",
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
            QMessageBox::critical(nullptr, "Fatal error", message);
            exit(1);
    }
    fflush(stdout);
}

static unique_ptr<QApplication> initializeQApp(int &argc, char **argv)
{
    // Put /usr/local/bin hardcoded to env
    auto usr_local_bin = QStringLiteral("/usr/local/bin");
    auto PATHS = QString(qgetenv("PATH")).split(':');
    if (!PATHS.contains(usr_local_bin))
        PATHS.prepend(usr_local_bin);
    auto PATH = PATHS.join(':').toLocal8Bit();
    qputenv("PATH", PATH);

    if (const char *key = "LANGUAGE"; qEnvironmentVariableIsSet(key))
        QLocale::setDefault(QLocale(qEnvironmentVariable(key)));
    else if (key = "LANG"; qEnvironmentVariableIsSet(key))
        QLocale::setDefault(QLocale(qEnvironmentVariable(key)));

    auto qapp = make_unique<QApplication>(argc, argv);
    QApplication::setApplicationName("albert");
    QApplication::setApplicationDisplayName("Albert");
    QApplication::setApplicationVersion(ALBERT_VERSION_STRING);
    albert::IconProvider ip;
    QSize size;
    QIcon icon(ip.getPixmap({"xdg:albert", "qrc:app_icon"}, &size, QSize(64, 64)));
    QApplication::setWindowIcon(QIcon(icon));
    QApplication::setQuitOnLastWindowClosed(false);

    // Create writable application paths
    if (auto path = albert::configLocation(); !QDir(path).mkpath("."))
        qFatal("Failed creating config dir at: %s", qPrintable(path));
    if (auto path = albert::dataLocation(); !QDir(path).mkpath("."))
        qFatal("Failed creating data dir at: %s", qPrintable(path));
    if (auto path = albert::cacheLocation(); !QDir(path).mkpath("."))
        qFatal("Failed creating cache dir at: %s", qPrintable(path));

    qInstallMessageHandler(messageHandler);

    return qapp;
}

int ALBERT_EXPORT main(int argc, char **argv);

int main(int argc, char **argv)
{
    QLoggingCategory::setFilterRules("*.debug=false");

    if (qApp != nullptr)
        qFatal("Calling main twice is not allowed.");

    auto qapp = initializeQApp(argc, argv);

    // https://doc.qt.io/qt-6/localization.html
    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale(), "qtbase", "_",
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QCoreApplication::installTranslator(&qtTranslator);
    }

    QTranslator translator;
    if (translator.load(QLocale(), qapp->applicationName(), "_", ":/i18n"))
        QCoreApplication::installTranslator(&translator);

    QCommandLineParser parser;
    auto opt_p = QCommandLineOption({"p", "plugin-dirs"}, "Set the plugin dirs to use. Comma separated.", "directory");
    auto opt_r = QCommandLineOption({"r", "report"}, "Print report and quit.");
    auto opt_n = QCommandLineOption({"n", "no-load"}, "Do not load enabled plugins.");
    parser.addOptions({opt_p, opt_r, opt_n});
    parser.addPositionalArgument("command", "RPC command to send to the running instance (Check 'albert commands')", "[command [params...]]");
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(
        "\nStart an Albert instance or control the running one.\n\n"
        "On Linux Wayland makes tons of problems use the XCB platform plugin instead. Like e.g.:\n"
        " QT_QPA_PLATFORM=xcb albert\n"
        " albert --platform xcb\n"
        "To get a detailed debug output including Qt logs use\n"
        " QT_LOGGING_RULES='*=true' albert\n"
        "See https://doc.qt.io/qt-6/qloggingcategory.html#logging-rules"
    );
    parser.process(*qapp);

    if (!parser.positionalArguments().isEmpty())
        return RPCServer::trySendMessage(parser.positionalArguments().join(" ")) ? 0 : 1;

    if (parser.isSet(opt_r))
        printReportAndExit();

    for (const auto &line : report())
        DEBG << line;

#if __has_include(<unistd.h>)
    UnixSignalHandler unix_signal_handler;
#endif

    app = new App(parser.value(opt_p).split(',', Qt::SkipEmptyParts), !parser.isSet(opt_n));
    app->initialize();
    int return_value = qApp->exec();
    app->finalize();
    app->deleteLater();
    QCoreApplication::processEvents(); // Never quit with events in queue

    if (return_value == -1 && runDetachedProcess(qApp->arguments(), QDir::currentPath()))
        return_value = EXIT_SUCCESS;

    INFO << "Bye.";
    return return_value;
}


///////////////////////////////////////////////////////////////////////////////


QNetworkAccessManager *albert::networkManager()
{ return &app->network_manager; }

std::unique_ptr<QSettings> albert::settings()
{ return make_unique<QSettings>(qApp->applicationName()); }

std::unique_ptr<QSettings> albert::state()
{ return make_unique<QSettings>(QString("%1/%2").arg(cacheLocation(), "albert.state"), QSettings::IniFormat); }

void albert::show(const QString &text)
{
    if (!text.isNull())
        app->frontend->setInput(text);
    app->frontend->setVisible(true);
}

void albert::hide()
{ app->frontend->setVisible(false); }

void albert::toggle()
{ app->frontend->isVisible() ? hide() : show(); }

QString albert::configLocation()
{ return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) ;}

QString albert::dataLocation()
{ return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) ;}

QString albert::cacheLocation()
{ return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) ;}

void albert::runTerminal(const QString &script, const QString &working_dir, bool close_on_exit)
{ app->terminal_provider.terminal().run(script, working_dir, close_on_exit); }

void albert::showSettings(QString plugin_id)
{
    if (!app->settings_window)
        app->settings_window = new SettingsWindow(*app);
    hide();
    app->settings_window->bringToFront(plugin_id);
}

void albert::restart()
{ QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1)); }

void albert::quit()
{ QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection); }

void albert::openUrl(const QString &url)
{ openUrl(QUrl(url)); }

void albert::openUrl(const QUrl &url)
{
    DEBG << QString("Opening URL '%1'").arg(url.toString());
    if (!QDesktopServices::openUrl(url))
        WARN << "Failed opening URL" << url;
}

void albert::openWebsite()
{
    static const char *website_url = "https://albertlauncher.github.io/";
    QDesktopServices::openUrl(QUrl(website_url));
}

void albert::openIssueTracker()
{
    static const char *issue_tracker_url = "https://github.com/albertlauncher/albert/issues";
    QDesktopServices::openUrl(QUrl(issue_tracker_url));
}

void albert::setClipboardText(const QString &text)
{
    QGuiApplication::clipboard()->setText(text, QClipboard::Clipboard);
    QGuiApplication::clipboard()->setText(text, QClipboard::Selection);
}

void albert::setClipboardTextAndPaste(const QString &text)
{
    setClipboardText(text);
#if defined Q_OS_MACOS
    runDetachedProcess({
        "osascript", "-e",
        R"(tell application "System Events" to keystroke "v" using command down)"
    });
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    if (qApp->platformName() == QStringLiteral("wayland")){
        QMessageBox::information(nullptr, qApp->applicationDisplayName(), "Pasting is not supported on wayland.");
    } else {
        QApplication::processEvents(); // ??
        auto *proc = new QProcess;
        proc->start("sh" , {"-c", "sleep 0.1 && xdotool key ctrl+v"});
        QObject::connect(proc, &QProcess::finished, proc, [proc](int exitCode, QProcess::ExitStatus exitStatus){
            if (exitStatus != QProcess::ExitStatus::NormalExit || exitCode != EXIT_SUCCESS){
                WARN << "Paste failed. xdotool installed?";
                QMessageBox::warning(nullptr, "Error", "Paste failed. xdotool installed?");
            }
            proc->deleteLater();
        });
    }
#elif defined Q_OS_WIN
Not implemented
#endif
}

long long albert::runDetachedProcess(const QStringList &commandline, const QString &working_dir)
{
    qint64 pid = 0;
    if (!commandline.empty()) {
        if (QProcess::startDetached(commandline[0], commandline.mid(1), working_dir.isNull() ? QDir::homePath() : working_dir, &pid))
            INFO << "Detached process started successfully. PID:" << pid << commandline;
        else
            WARN << "Starting detached process failed." << commandline;
    } else
        WARN << "runDetachedProcess: commandline must not be empty!";
    return pid;
}
