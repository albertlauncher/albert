// Copyright (c) 2022 Manuel Schneider

#include "app.h"
#include "config.h"
#include "export.h"
#include "logging.h"
#include "scopedcrashindicator.hpp"
#include "xdg/iconlookup.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTime>
#include <csignal>
#include <functional>
ALBERT_LOGGING
using namespace std;

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

static void printSystemReportAndExit()
{
    QTextStream out(stdout);
    auto print = [&out](const QString& s){ out << s << '\n'; };

    const uint8_t w = 22;
    print(QString("%1: %2").arg("Albert version", w).arg(QApplication::applicationVersion()));
    print(QString("%1: %2").arg("Build date", w).arg(__DATE__ " " __TIME__));
    print(QString("%1: %2").arg("Qt version", w).arg(qVersion()));
    print(QString("%1: %2").arg("QT_QPA_PLATFORMTHEME", w).arg(QString::fromLocal8Bit(qgetenv("QT_QPA_PLATFORMTHEME"))));
    print(QString("%1: %2").arg("Binary location", w).arg(QApplication::applicationFilePath()));
    print(QString("%1: %2").arg("PWD", w).arg(QString::fromLocal8Bit(qgetenv("PWD"))));
    print(QString("%1: %2").arg("SHELL", w).arg(QString::fromLocal8Bit(qgetenv("SHELL"))));
    print(QString("%1: %2").arg("LANG", w).arg(QString::fromLocal8Bit(qgetenv("LANG"))));
#if defined(Q_OS_LINUX)
    print(QString("%1: %2").arg("XDG_SESSION_TYPE", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE"))));
    print(QString("%1: %2").arg("XDG_CURRENT_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"))));
    print(QString("%1: %2").arg("DESKTOP_SESSION", w).arg(QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION"))));
    print(QString("%1: %2").arg("XDG_SESSION_DESKTOP", w).arg(QString::fromLocal8Bit(qgetenv("XDG_SESSION_DESKTOP"))));
#endif
    print(QString("%1: %2").arg("OS", w).arg(QSysInfo::prettyProductName()));
    print(QString("%1: %2/%3").arg("OS (type/version)", w).arg(QSysInfo::productType(), QSysInfo::productVersion()));
    print(QString("%1: %2").arg("Build ABI", w).arg(QSysInfo::buildAbi()));
    print(QString("%1: %2/%3").arg("Arch (build/current)", w).arg(QSysInfo::buildCpuArchitecture(), QSysInfo::currentCpuArchitecture()));
    print(QString("%1: %2/%3").arg("Kernel (type/version)", w).arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()));

    out.flush();
    ::exit(EXIT_SUCCESS);
}

int ALBERT_EXPORT main(int argc, char **argv);
int main(int argc, char **argv)
{
    if (qApp != nullptr)
        qFatal("Calling main twice is not allowed.");

    ScopedCrashIndicator crash_indicator;

    qInstallMessageHandler(messageHandler);

    //    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication qapp(argc, argv);
    qApp->setOrganizationName("albert");
    qApp->setApplicationName("albert");
    qApp->setApplicationDisplayName("Albert");
    qApp->setApplicationVersion(ALBERT_VERSION);
    QString icon = XDG::IconLookup::iconPath("albert");
    if (icon.isEmpty()) icon = ":app_icon";
    qApp->setWindowIcon(QIcon(icon));
    qApp->setQuitOnLastWindowClosed(false);

    QSettings::setPath(QSettings::defaultFormat(), QSettings::UserScope,
                       QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

    // Create writable application paths
    auto locs = {
            QStandardPaths::AppConfigLocation,
            QStandardPaths::AppDataLocation,
            QStandardPaths::CacheLocation
    };
    for (auto loc: locs)
        if (auto path = QStandardPaths::writableLocation(loc); !QDir(path).mkpath("."))
            qFatal("Could not create dir: %s", qPrintable(path));

#if defined(Q_OS_MAC)
    //setActivationPolicyAccessory();
#endif

    // Install signal handlers
    for (int sig: {SIGINT, SIGTERM, SIGHUP, SIGPIPE})
        signal(sig, [](int) { QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection); });

    QCommandLineParser parser;
    parser.setApplicationDescription("These options may change in future versions.");
    auto opt_p = QCommandLineOption({"p", "plugin-dirs"}, "Set the plugin dirs to use. Comma separated.", "directory");
    auto opt_r = QCommandLineOption({"r", "report"}, "Print issue report.");
    auto opt_q = QCommandLineOption({"q", "quiet"}, "Warnings only.");
    auto opt_d = QCommandLineOption({"d", "debug"}, "Full debug output. Ignore '--quiet'.");
    parser.addOptions({opt_p, opt_r, opt_q, opt_d});
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(*qApp);

    if (parser.isSet(opt_r))
        printSystemReportAndExit();
    else if (parser.isSet(opt_q))
        QLoggingCategory::setFilterRules("*.debug=false\n*.info=false");
    else if (!parser.isSet(opt_d))
        QLoggingCategory::setFilterRules("*.debug=false");

    auto albert_app = new App(parser.isSet(opt_p) ? parser.value(opt_p).split(',') : QStringList());

    // Delete app _before_ eventloop exits
    QObject::connect(qApp, &QApplication::aboutToQuit, [&](){ delete albert_app; });

    int return_value = qApp->exec();
    if (return_value == -1) {
        qint64 pid;
        if (QProcess::startDetached(qApp->arguments()[0], qApp->arguments().mid(1), QString(), &pid))
            INFO << "Application restarted. PID" << pid;
        else
            WARN << "Restarting process failed";
        return 0;
    }

    return return_value;
}
