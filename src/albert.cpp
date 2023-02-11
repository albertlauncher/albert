// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/config.h"
#include "albert/export.h"
#include "albert/extensions/frontend.h"
#include "albert/logging.h"
#include "albert/util/util.h"
#include "app.h"
#include "rpcserver.h"
#include "scopedcrashindicator.h"
#include "xdg/iconlookup.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QMessageBox>
#include <QMetaEnum>
#include <QSettings>
#include <QStandardPaths>
#include <QTime>
#include <csignal>
#include <memory>
#ifdef Q_OS_MAC
#include "macos.h"
#endif
ALBERT_LOGGING
using namespace std;
using namespace albert;

namespace {
const char *CFG_LAST_USED_VERSION = "last_used_version";
unique_ptr<App> app;
}

void albert::show(const QString &text)
{
    if (!text.isNull())
        app->plugin_provider.frontend()->setInput(text);
    app->plugin_provider.frontend()->setVisible(true);
}

void albert::hide()
{
    app->plugin_provider.frontend()->setVisible(false);
}

void albert::toggle()
{
    app->plugin_provider.frontend()->setVisible(!app->plugin_provider.frontend()->isVisible());
}

void albert::runTerminal(const QString &script, const QString &working_dir, bool close_on_exit)
{
    app->terminal_provider.terminal().run(script, working_dir, close_on_exit);
}

void albert::showSettings()
{
    hide();
    if (!app->settings_window)
        app->settings_window = new SettingsWindow(*app);
    app->settings_window->bringToFront();
}

void albert::restart()
{
    QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1));
}

void albert::quit()
{
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

void albert::sendTrayNotification(const QString &title, const QString &message, int ms)
{
    app->tray_icon.showMessage(title, message, QSystemTrayIcon::NoIcon, ms);
}


static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
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

static void createWritableApplicationPaths()
{
    auto locs = {
            QStandardPaths::AppConfigLocation,
            QStandardPaths::AppDataLocation,
            QStandardPaths::CacheLocation
    };
    for (auto loc: locs)
        if (auto path = QStandardPaths::writableLocation(loc); !QDir(path).mkpath("."))
            qFatal("Could not create dir: %s", qPrintable(path));
}

static void installSignalHandlers()
{
    for (int sig: {SIGINT, SIGTERM, SIGHUP, SIGPIPE})
        signal(sig, [](int) { QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection); });
}

static unique_ptr<QApplication> initializeQApp(int &argc, char **argv)
{
    if (const char *key = "LANGUAGE"; qEnvironmentVariableIsSet(key))
        QLocale::setDefault(QLocale(qEnvironmentVariable(key)));
    else if (key = "LANG"; qEnvironmentVariableIsSet(key))
        QLocale::setDefault(QLocale(qEnvironmentVariable(key)));

    auto qapp = make_unique<QApplication>(argc, argv);
    QApplication::setApplicationName("albert");
    QApplication::setApplicationDisplayName("Albert");
    QApplication::setApplicationVersion(ALBERT_VERSION);
    QString icon = XDG::IconLookup::iconPath("albert");
    if (icon.isEmpty())
        icon = ":app_icon";
    QApplication::setWindowIcon(QIcon(icon));
    QApplication::setQuitOnLastWindowClosed(false);

    installSignalHandlers();
    createWritableApplicationPaths();

    return qapp;
}

static void printSystemReport()
{
    QTextStream out(stdout);
    auto print = [&out](const QString& s){ out << s << '\n'; };

    const uint8_t w = 22;
    print(QString("%1: %2").arg("Albert version", w).arg(QApplication::applicationVersion()));
    print(QString("%1: %2").arg("Build date", w).arg(__DATE__ " " __TIME__));
    print(QString("%1: %2").arg("Qt version", w).arg(qVersion()));
    print(QString("%1: %2").arg("Build ABI", w).arg(QSysInfo::buildAbi()));
    print(QString("%1: %2/%3").arg("Arch (build/current)", w).arg(QSysInfo::buildCpuArchitecture(), QSysInfo::currentCpuArchitecture()));
    print(QString("%1: %2/%3").arg("Kernel (type/version)", w).arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()));
    print(QString("%1: %2").arg("OS", w).arg(QSysInfo::prettyProductName()));
    print(QString("%1: %2/%3").arg("OS (type/version)", w).arg(QSysInfo::productType(), QSysInfo::productVersion()));
    print(QString("%1: %2").arg("$QT_QPA_PLATFORMTHEME", w).arg(QString::fromLocal8Bit(qgetenv("QT_QPA_PLATFORMTHEME"))));
    print(QString("%1: %2").arg("Platform name", w).arg(QGuiApplication::platformName()));
    print(QString("%1: %2").arg("Font", w).arg(QGuiApplication::font().toString()));
    print(QString("%1: %2").arg("Binary location", w).arg(QApplication::applicationFilePath()));
    print(QString("%1: %2").arg("$PWD", w).arg(QString::fromLocal8Bit(qgetenv("PWD"))));
    print(QString("%1: %2").arg("$SHELL", w).arg(QString::fromLocal8Bit(qgetenv("SHELL"))));
    print(QString("%1: %2").arg("$LANG", w).arg(QString::fromLocal8Bit(qgetenv("LANG"))));
    QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Language>();
    QLocale loc;
    print(QString("%1: %2").arg("Language", w).arg(metaEnum.valueToKey(loc.language())));
    print(QString("%1: %2").arg("Locale", w).arg(loc.name()));
#if defined(Q_OS_LINUX)
    print(QString("%1: %2").arg("$XDG_SESSION_TYPE", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE"))));
    print(QString("%1: %2").arg("$XDG_CURRENT_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"))));
    print(QString("%1: %2").arg("$DESKTOP_SESSION", w).arg(QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION"))));
    print(QString("%1: %2").arg("$XDG_SESSION_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_DESKTOP"))));
    print(QString("%1: %2").arg("Icon theme", w).arg(QIcon::themeName()));
#endif
    out.flush();
}

static void notifyVersionChange()
{
    auto settings = QSettings(qApp->applicationName());
    auto current_version = qApp->applicationVersion();
    auto last_used_version = settings.value(CFG_LAST_USED_VERSION).toString();

    if (last_used_version.isNull()){  // First run
        QMessageBox(
                QMessageBox::Warning, "First run",
                "This is the first time you've launched Albert. Albert is plugin based. "
                "You have to enable some plugins you want to use.").exec();
        albert::showSettings();
    }
    else if (current_version.section('.', 1, 1) != last_used_version.section('.', 1, 1) )  // FIXME in first major version
        QMessageBox(QMessageBox::Information, "Major version changed",
                    QString("You are now using Albert %1. The major version changed. "
                            "Some parts of the API might have changed. Check the "
                            "<a href=\"https://albertlauncher.github.io/news/\">news</a>.")
                            .arg(current_version)).exec();

    if (last_used_version != current_version)
        settings.setValue(CFG_LAST_USED_VERSION, current_version);
}

int ALBERT_EXPORT main(int argc, char **argv);
int main(int argc, char **argv)
{
    if (qApp != nullptr)
        qFatal("Calling main twice is not allowed.");

    qInstallMessageHandler(messageHandler);
    auto qapp = initializeQApp(argc, argv);

    QCommandLineParser parser;
    auto opt_p = QCommandLineOption({"p", "plugin-dirs"}, "Set the plugin dirs to use. Comma separated.", "directory");
    auto opt_r = QCommandLineOption({"r", "report"}, "Print issue report.");
    auto opt_q = QCommandLineOption({"q", "quiet"}, "Warnings only.");
    auto opt_d = QCommandLineOption({"d", "debug"}, "Full debug output. Ignore '--quiet'.");
    parser.addOptions({opt_p, opt_r, opt_q, opt_d});
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

    if (parser.isSet(opt_q))
        QLoggingCategory::setFilterRules("*.debug=false\n*.info=false");
    else if (parser.isSet(opt_d))
        printSystemReport();
    else
        QLoggingCategory::setFilterRules("*.debug=false");

#if defined(Q_OS_MAC)
    setActivationPolicyAccessory();
#endif
    ScopedCrashIndicator crash_indicator;
    app = make_unique<App>(parser.value(opt_p).split(',', Qt::SkipEmptyParts));
    app->initialize();
    notifyVersionChange();
    QObject::connect(qApp, &QApplication::aboutToQuit, [&]() { app.reset(); }); // Delete app _before_ loop exits

//    albert::showSettings();

    int return_value = qApp->exec();
    if (return_value == -1 && runDetachedProcess(qApp->arguments(), QDir::currentPath()))
        return EXIT_SUCCESS;
    return return_value;
}
