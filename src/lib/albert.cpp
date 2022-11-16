// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/config.h"
#include "albert/export.h"
#include "albert/extensionregistry.h"
#include "albert/extensions/frontend.h"
#include "albert/util/standarditem.h"
#include "albert/util/util.h"
#include "albert/extensions/configwidgetprovider.h"
#include "albert/logging.h"
#include "pluginprovider.h"
#include "usagehistory.h"
#include "queryengine.h"
#include "rpcserver.h"
#include "scopedcrashindicator.hpp"
#include "settings/settingswindow.h"
#include "terminalprovider.h"
#include "xdg/iconlookup.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
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
unique_ptr<albert::ExtensionRegistry> extension_registry;
unique_ptr<QueryEngine> query_engine;
unique_ptr<::PluginProvider> plugin_provider;
unique_ptr<TerminalProvider> terminal_provider;
QPointer<SettingsWindow> settings_window;

class : public albert::ConfigWidgetProvider,
        public albert::IndexQueryHandler

{
    struct CongigWidgetGeneral: public QWidget { Ui::General ui; };


    QString id() const override { return "albert"; }

    // ConfigWidgetProvider
    QWidget* buildConfigWidget() override {
        auto *w = new CongigWidgetGeneral;
        auto &ui = w->ui;
        ui.setupUi(w);

        for (const auto &spec : plugin_provider->frontends()){
            ui.comboBox_frontend->addItem(spec.name);
            if (spec.id == plugin_provider->frontend->id())
                ui.comboBox_frontend->setCurrentIndex(ui.comboBox_frontend->count()-1);
        }

        QObject::connect(ui.comboBox_frontend, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                         [](int index){ plugin_provider->setFrontend(index); });

        for (const auto &terminal : terminal_provider->terminals()){
            ui.comboBox_term->addItem(terminal->name());
            if (terminal.get() == &terminal_provider->terminal())
                ui.comboBox_term->setCurrentIndex(ui.comboBox_term->count()-1);
        }

        QObject::connect(ui.comboBox_term, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                         [](int index){ terminal_provider->setTerminal(index); });

        return w;
    }
    QString configTitle() const override { return QObject::tr("General"); }
    ConfigGroup configGroup() const override { return ConfigGroup::General; }

    // IndexQueryHandler
    std::vector<IndexItem> indexItems() const override
    {
        auto settings_item = make_shared<StandardItem>(
                "albert-settings", "Settings", "Open the Albert settings window", QStringList{":app_icon"},
                Actions{{"albert-settings", "Open settings", [](){ albert::showSettings(); }}}
        );

        auto quit_item = make_shared<StandardItem>(
                "albert-quit", "Quit Albert", "Quit this application", QStringList{":app_icon"},
                Actions{{"albert-quit", "Quit Albert", [](){ albert::quit(); }}}
        );

        auto restart_item = make_shared<StandardItem>(
                "albert-restart", "Restart Albert", "Restart this application", QStringList{":app_icon"},
                Actions{{"albert-restart", "Restart Albert", [](){ albert::restart(); }}}
        );

        return {
                {settings_item, "settings"},
                {settings_item, "preferences"},
                {quit_item, "quit"},
                {restart_item, "restart"}
        };
    }
} core_extension;

}

albert::ExtensionRegistry &albert::extensionRegistry()
{
    return *extension_registry;
}

void albert::show(const QString &text)
{
    if (!text.isNull())
        plugin_provider->frontend->setInput(text);
    plugin_provider->frontend->setVisible(true);
}

void albert::hide()
{
    plugin_provider->frontend->setVisible(false);
}

void albert::toggle()
{
    plugin_provider->frontend->setVisible(!plugin_provider->frontend->isVisible());
}

void albert::runTerminal(const QString &script, const QString &working_dir, bool close_on_exit)
{
    terminal_provider->terminal().run(script, working_dir, close_on_exit);
}

void albert::showSettings()
{
    if (!settings_window)
        settings_window = new SettingsWindow(*extension_registry);
    settings_window->bringToFront();
}

void albert::restart()
{
    QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1));
}

void albert::quit()
{
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
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
    auto qapp = make_unique<QApplication>(argc, argv);
    QApplication::setOrganizationName("albert");
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

static QStringList defaultPluginDirs()
{
    QStringList pluginDirs;
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
    pluginDirs.push_back(QDir("../lib").canonicalPath()); // TODO deplopyment?
#elif defined _WIN32
    qFatal("Not implemented");
#endif
    return pluginDirs;
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

static void notifyVersionChange()
{
    auto settings = QSettings();
    auto current_version = qApp->applicationVersion();
    auto last_used_version = settings.value(CFG_LAST_USED_VERSION).toString();

    if (last_used_version.isNull()){  // First run
        QMessageBox(
                QMessageBox::Warning, "First run",
                "This is the first time you've launched Albert. Albert is plugin based. "
                "You have to enable extension you want to use. "
                "Note that you wont be able to open albert without a hotkey or "
                "tray icon.").exec();
        albert::showSettings();
        settings.setValue(CFG_LAST_USED_VERSION, current_version);
    }
    else if (current_version.section('.', 1, 1) != last_used_version.section('.', 1, 1) ){  // FIXME in first major version
        QMessageBox(QMessageBox::Information, "Major version changed",
                    QString("You are now using Albert %1. The major version changed. "
                            "Some parts of the API might have changed. Check the "
                            "<a href=\"https://albertlauncher.github.io/news/\">news</a>.")
                            .arg(current_version)).exec();
    }
}

int ALBERT_EXPORT main(int argc, char **argv);
int main(int argc, char **argv)
{
    if (qApp != nullptr)
        qFatal("Calling main twice is not allowed.");

    qInstallMessageHandler(messageHandler);
    ScopedCrashIndicator crash_indicator;
    auto app = initializeQApp(argc, argv);

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

#if defined(Q_OS_MAC)
    setActivationPolicyAccessory();
#endif

    RPCServer rpc_server;  // Todo > plugin
    extension_registry = std::make_unique<albert::ExtensionRegistry>();
    query_engine = std::make_unique<QueryEngine>(*extension_registry);
    plugin_provider = std::make_unique<::PluginProvider>(*extension_registry);
    terminal_provider = std::make_unique<TerminalProvider>();
    extension_registry->add(plugin_provider.get());
    UsageHistory::initializeDatabase();
    notifyVersionChange();

    plugin_provider->findPlugins(defaultPluginDirs() << parser.value(opt_p).split(','));
    plugin_provider->loadPlugins();
    plugin_provider->frontend->setEngine(query_engine.get());
    QObject::connect(qApp, &QApplication::aboutToQuit,
                     [&]() { plugin_provider->unloadPlugins(); }); // Delete app _before_ loop exits

    albert::showSettings();
    int return_value = qApp->exec();
    if (return_value == -1 && runDetachedProcess(qApp->arguments(), QDir::currentPath()))
        return EXIT_SUCCESS;
    return return_value;
}