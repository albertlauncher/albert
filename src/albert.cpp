// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/config.h"
#include "albert/logging.h"
#include "albert/util/iconprovider.h"
#include "app.h"
#include <QApplication>
#include <QClipboard>
#include <QCommandLineParser>
#include <QDesktopServices>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QMetaEnum>
#include <QProcess>
#include <QSettings>
#include <QStyleFactory>
#include <QStandardPaths>
#include <QTime>
#if __has_include(<unistd.h>)
#include "platform/Unix/unixsignalhandler.h"
#endif
ALBERT_LOGGING_CATEGORY("albert")
using namespace std;
using namespace albert;
static App *app;


static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    // Todo use std::format as soon as apple gets it off the ground
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

static void printSystemReport()
{
    const uint8_t w = 21;

    // BUILD
    DEBG << QString("%1: %2").arg("Albert version", w).arg(QApplication::applicationVersion());
    DEBG << QString("%1: %2").arg("Build date", w).arg(__DATE__ " " __TIME__);
    DEBG << QString("%1: %2").arg("Qt version", w).arg(qVersion());
    DEBG << QString("%1: %2").arg("Build ABI", w).arg(QSysInfo::buildAbi());
    DEBG << QString("%1: %2/%3").arg("Arch (build/current)", w).arg(QSysInfo::buildCpuArchitecture(), QSysInfo::currentCpuArchitecture());

    // SYSTEM/PLATFORM
    DEBG << QString("%1: %2/%3").arg("Kernel (type/version)", w).arg(QSysInfo::kernelType(), QSysInfo::kernelVersion());
    DEBG << QString("%1: %2").arg("OS", w).arg(QSysInfo::prettyProductName());
    DEBG << QString("%1: %2/%3").arg("OS (type/version)", w).arg(QSysInfo::productType(), QSysInfo::productVersion());
    DEBG << QString("%1: %2").arg("Platform name", w).arg(QGuiApplication::platformName());
    DEBG << QString("%1: %2").arg("Style name", w).arg(QApplication::style()->objectName());
    DEBG << QString("%1: %2").arg("Available styles", w).arg(QStyleFactory::keys().join(", "));

    // APP/QT
    DEBG << QString("%1: %2").arg("Binary location", w).arg(QApplication::applicationFilePath());
    DEBG << QString("%1: %2").arg("Current dir", w).arg(QDir::currentPath());
    DEBG << QString("%1: %2").arg("Font", w).arg(QGuiApplication::font().toString());
    QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Language>();
    QLocale loc;
    DEBG << QString("%1: %2").arg("Language", w).arg(metaEnum.valueToKey(loc.language()));
    DEBG << QString("%1: %2").arg("Locale", w).arg(loc.name());

    // ENV
    DEBG << QString("%1: %2").arg("$LANG", w).arg(QString::fromLocal8Bit(qgetenv("LANG")));
    DEBG << QString("%1: %2").arg("$QT_QPA_PLATFORMTHEME", w).arg(QString::fromLocal8Bit(qgetenv("QT_QPA_PLATFORMTHEME")));
    DEBG << QString("%1: %2").arg("$PATH", w).arg(QString::fromLocal8Bit(qgetenv("PATH")));
    DEBG << QString("%1: %2").arg("$PWD", w).arg(QString::fromLocal8Bit(qgetenv("PWD")));
    DEBG << QString("%1: %2").arg("$SHELL", w).arg(QString::fromLocal8Bit(qgetenv("SHELL")));

    // LINUX ENV
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    DEBG << QString("%1: %2").arg("$XDG_SESSION_TYPE", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE")));
    DEBG << QString("%1: %2").arg("$XDG_CURRENT_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")));
    DEBG << QString("%1: %2").arg("$DESKTOP_SESSION", w).arg(QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION")));
    DEBG << QString("%1: %2").arg("$XDG_SESSION_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_DESKTOP")));
    DEBG << QString("%1: %2").arg("Icon theme", w).arg(QIcon::themeName());
#endif
}

int ALBERT_EXPORT main(int argc, char **argv);

int main(int argc, char **argv)
{
    if (qApp != nullptr)
        qFatal("Calling main twice is not allowed.");

    auto qapp = initializeQApp(argc, argv);

    QCommandLineParser parser;
    auto opt_p = QCommandLineOption({"p", "plugin-dirs"}, "Set the plugin dirs to use. Comma separated.", "directory");
    auto opt_r = QCommandLineOption({"r", "report"}, "Print issue report.");
    auto opt_d = QCommandLineOption({"d", "debug"}, "Full debug output.");
    auto opt_q = QCommandLineOption({"q", "quiet"}, "Warnings only.  Takes precedence over -d.");
    auto opt_l = QCommandLineOption({"l", "loggin-rules"}, "QLoggingCategory filter rules. Takes precedence over -q.", "rules");
    parser.addOptions({opt_p, opt_r, opt_q, opt_d, opt_l});
    parser.addPositionalArgument("command", "RPC command to send to the running instance", "[command [params...]]");
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(*qapp);

    if (parser.isSet(opt_r)){
        printSystemReport();
        ::exit(EXIT_SUCCESS);
    }

    if (!parser.positionalArguments().isEmpty())
        RPCServer::trySendMessageAndExit(parser.positionalArguments().join(" "));

    if (parser.isSet(opt_l)){
        QLoggingCategory::setFilterRules(parser.value(opt_l));
    } else if (parser.isSet(opt_q)){
        QLoggingCategory::setFilterRules("*.debug=false\n*.info=false");
    } else if (parser.isSet(opt_d)){
        printSystemReport();
        QLoggingCategory::setFilterRules("");
    } else
        QLoggingCategory::setFilterRules("*.debug=false");

#if __has_include(<unistd.h>)
    UnixSignalHandler unix_signal_handler;
#endif
    app = new App(parser.value(opt_p).split(',', Qt::SkipEmptyParts));
    QTimer::singleShot(0, qApp, [](){ app->initialize(); }); // Init with running eventloop
    QObject::connect(qApp, &QApplication::aboutToQuit, [&]() { delete app; }); // Delete app _before_ loop exits

    int return_value = qApp->exec();
    if (return_value == -1 && runDetachedProcess(qApp->arguments(), QDir::currentPath()))
        return_value = EXIT_SUCCESS;

    INFO << "Bye.";

    return return_value;
}


///////////////////////////////////////////////////////////////////////////////


QNetworkAccessManager *albert::networkManager()
{ return &app->network_manager; }

std::unique_ptr<QSettings> albert::settings()
{ return make_unique<QSettings>(qApp->applicationName(), qApp->applicationName()); }

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
        QApplication::processEvents();
        if (0 > runDetachedProcess({"sh", "-c", "sleep 0.1 && xdotool key ctrl+v"})){
            QString msg = "Failed running xdotool. Is it installed?";
            WARN << msg;
            QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
        }
    }
#elif defined Q_OS_WIN
Not implemented
#endif
}

long long albert::runDetachedProcess(const QStringList &commandline, const QString &working_dir)
{
    qint64 pid = 0;
    if (!commandline.empty()) {
        if (QProcess::startDetached(commandline[0], commandline.mid(1), working_dir, &pid))
            INFO << "Detached process started successfully. PID:" << pid << commandline;
        else
            WARN << "Starting detached process failed." << commandline;
    } else
        WARN << "runDetachedProcess: commandline must not be empty!";
    return pid;
}
