// Copyright (c) 2023-2025 Manuel Schneider

#include "application.h"
#include "config.h"
#include "extensionregistry.h"
#include "frontend.h"
#include "frontendregistry.h"
#include "hotkeymanager.h"
#include "localization.h"
#include "logging.h"
#include "messagebox.h"
#include "messagehandler.h"
#include "pathmanager.h"
#include "platform.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
#include "qtpluginprovider.h"
#include "queryengine.h"
#include "report.h"
#include "rpcserver.h"
#include "session.h"
#include "settingswindow.h"
#include "signalhandler.h"
#include "systemtrayicon.h"
#include "systemutil.h"
#include "telemetry.h"
#include "triggersqueryhandler.h"
#include "urldispatcher.h"
#include "urlhandler.h"
#include <QCommandLineParser>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <iostream>
Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert")
using namespace Qt::StringLiterals;
using namespace albert::detail;
using namespace albert;
using namespace std;


namespace {
App *app_instance = nullptr;
static const char *STATE_LAST_USED_VERSION = "last_used_version";
}

App &albert::app() { return *app_instance; }

// -------------------------------------------------------------------------------------------------

App::App()
{
    if (app_instance)
        qFatal("There can be only one app instance.");
    app_instance = this;
}

App::~App() { app_instance = nullptr; }

void App::restart()
{ QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1)); }

void App::quit()
{ QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection); }

const filesystem::path &App::cacheLocation()
{
    static const auto path = filesystem::path(
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation).toStdString());
    return path;
}

const filesystem::path &App::configLocation()
{
    static const auto path = filesystem::path(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString());
    return path;
}

const filesystem::path &App::dataLocation()
{
    static const auto path = filesystem::path(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString());
    return path;
}

unique_ptr<QSettings> App::settings()
{
    return make_unique<QSettings>(
        QString::fromStdString((configLocation() / "config").string()),
        QSettings::IniFormat
    );
}

unique_ptr<QSettings> App::state()
{
    return make_unique<QSettings>(
        QString::fromStdString((dataLocation() / "state").string()),
        QSettings::IniFormat
    );
}

// -------------------------------------------------------------------------------------------------

class Application::Private
{
public:

    Private(Application &app,
            const QStringList &additional_plugin_paths,
            bool load_enabled,
            QSettings &settings,
            QSettings &state);
    ~Private();

    void initRPC();

    QString loadFrontend(albert::PluginLoader *);
    void notifyVersionChange(QSettings &state);

public:

    Application &app;

    // As early as possible
    RPCServer rpc_server; // Check for other instances first
    SignalHandler unix_signal_handler;
    PathManager path_manager;
    Localization localization;

    // Core
    albert::ExtensionRegistry extension_registry;
    PluginRegistry plugin_registry;
    QtPluginProvider plugin_provider;
    FrontendRegistry frontend_registry;
    Frontend &frontend;  // convenience reference
    QueryEngine query_engine;
    Telemetry telemetry;
    SystemTrayIcon tray_icon;
    HotkeyManager hotkey_manager;
    UrlDispatcher url_dispatcher;

    // Weak, lazy or optional
    std::unique_ptr<Session> session{nullptr};
    QPointer<SettingsWindow> settings_window{nullptr};

    PluginQueryHandler plugin_query_handler;
    TriggersQueryHandler triggers_query_handler;

};


Application::Private::Private(Application &q,
                              const QStringList &additional_plugin_paths,
                              bool load_enabled,
                              QSettings &settings,
                              QSettings &state):
    app(q),
    path_manager(settings),
    localization(settings),
    plugin_registry(extension_registry, load_enabled),
    plugin_provider(additional_plugin_paths),
    frontend_registry(settings, plugin_provider),
    frontend(frontend_registry.frontend()),
    query_engine(extension_registry),
    telemetry(plugin_registry, extension_registry),
    tray_icon(settings, frontend),
    hotkey_manager(settings),
    plugin_query_handler(plugin_registry),
    triggers_query_handler(query_engine)
{
    platform::initPlatform();
    platform::initNativeWindow(frontend.winId());


    connect(&frontend, &Frontend::visibleChanged,
            &app, [this]{
                if (frontend.isVisible())
                    session = make_unique<Session>(query_engine, frontend);
                else
                    session.reset();
            });

    auto reset_session = [this] {
        if (frontend.isVisible()) {
            session.reset();  // Make sure session is deleted _before_ creating a new one
            session = make_unique<Session>(query_engine, frontend);
        }
    };

    connect(&query_engine, &QueryEngine::queryHandlerAdded,
            &app, reset_session);

    connect(&query_engine, &QueryEngine::queryHandlerRemoved,
            &app, reset_session, Qt::QueuedConnection);

    connect(&hotkey_manager, &HotkeyManager::activated,
            &app, &Application::toggle);

    initRPC(); // Also may trigger frontend

    notifyVersionChange(state);

    for (auto *ext : frontend.extensions())
        extension_registry.registerExtension(ext);
    extension_registry.registerExtension(&plugin_query_handler);
    extension_registry.registerExtension(&triggers_query_handler);

    // Load plugins not before loop is executing
    QTimer::singleShot(0, [this] { extension_registry.registerExtension(&plugin_provider); });
}

Application::Private::~Private()
{

    delete settings_window.get();
    session.reset();

    extension_registry.deregisterExtension(&plugin_provider);  // unloads plugins
    extension_registry.deregisterExtension(&triggers_query_handler);
    extension_registry.deregisterExtension(&plugin_query_handler);
}

void Application::Private::initRPC()
{
    auto messageHandler = [this](const QByteArray bytes) -> QByteArray
    {
        INFO << "Received RPC message:" << bytes;

        const auto array = QJsonDocument::fromJson(bytes).array();

        QStringList args;
        for (const QJsonValue &value : array)
            args << value.toString();

        if (args.size() == 0)
        {
            WARN << "Received Invalid message expected json array of strings.";
            return "Invalid message expected json array of strings.";
        }

        else if (args[0] == "show")
        {
            if (args.size() > 2)
                return "'show' expects zero or one argument.";

            else if (args.size() == 2)
                app.show(args[1]);

            else // if (args.size() == 1)
                app.show();
        }

        else if (args[0] == "hide")

            if (args.size() == 1)
                app.hide();
            else
                return "'hide' expects no arguments.";

        else if (args[0] == "toggle")

            if (args.size() == 1)
                app.toggle();
            else
                return "'toggle' expects no arguments.";

        else if (args[0] == "settings")
        {
            if (args.size() > 2)
                return "'settings' expects zero or one argument.";

            else if (args.size() == 2)
                app.showSettings(args[1]);

            else // if (args.size() == 1)
                app.showSettings();
        }

        else if (args[0] == "restart")

            if (args.size() == 1)
                app.restart();
            else
                return "'restart' expects no arguments.";

        else if (args[0] == "quit")

            if (args.size() == 1)
                app.quit();
            else
                return "'quit' expects no arguments.";

        else if (args[0] == "report")

            if (args.size() == 1)
                return report().join('\n').toLocal8Bit();
            else
                return "'report' expects no arguments.";

        else if (ranges::all_of(args, [](const auto &arg){
                     QUrl url(arg);
                     return url.isValid() && url.scheme() == qApp->applicationName();
                 }))
            for (const auto &arg : as_const(args))
                url_dispatcher.dispatch(arg);

        else
        {
            const auto *msg = "Invalid RPC message";
            WARN << msg << bytes;
            return msg;
        }

        return {};
    };

    rpc_server.setMessageHandler(messageHandler);
}

void Application::Private::notifyVersionChange(QSettings &state)
{
    auto current_version = qApp->applicationVersion();
    auto last_used_version = state.value(STATE_LAST_USED_VERSION).toString();

    // First run
    if (last_used_version.isNull())
    {
        auto text = tr("This is the first time you've launched Albert. Albert is "
                       "plugin based. You have to enable some plugins you want to use.");

        information(text);

        QTimer::singleShot(0, &app, [&]{ app.showSettings(); });
    }
    else if (current_version.section('.', 0, 0) != last_used_version.section('.', 0, 0))
    {
        auto text = tr("You are now using Albert %1. The major version changed. "
                       "Some parts of the API might have changed. "
                       "Check the <a href=\"https://albertlauncher.github.io/news/\">news</a>."
                       ).arg(current_version);

        information(text);
    }

    if (last_used_version != current_version)
        state.setValue(STATE_LAST_USED_VERSION, current_version);
}

// -------------------------------------------------------------------------------------------------

Application::Application(const QStringList &additional_plugin_paths, bool load_enabled) :
    d(make_unique<Private>(*this,
                           additional_plugin_paths,
                           load_enabled,
                           *App::settings(),
                           *App::state()))
{
    connect(&d->extension_registry, &ExtensionRegistry::added, this, &Application::added);
    connect(&d->extension_registry, &ExtensionRegistry::removed, this, &Application::removed);
}

Application::~Application() {}

int Application::run(const QStringList &additional_plugin_paths, bool load_enabled)
{
    Application app(additional_plugin_paths, load_enabled);
    return qApp->exec();
}


PluginRegistry &Application::pluginRegistry() { return d->plugin_registry; }

FrontendRegistry &Application::frontenRegistry() { return d->frontend_registry; }

QueryEngine &Application::queryEngine() { return d->query_engine; }

Telemetry &Application::telemetry() { return d->telemetry; }

HotkeyManager &Application::hotkeyManager() { return d->hotkey_manager; }

SystemTrayIcon &Application::systemTrayIcon() { return d->tray_icon; }

PathManager &Application::pathManager() { return d->path_manager; }

Localization &Application::localization() { return d->localization; }

const map<QString, Extension *> &Application::extensions() const
{ return d->extension_registry.extensions(); }

void Application::showSettings(QString plugin_id)
{
    if (!d->settings_window)
        d->settings_window = new SettingsWindow(d->frontend_registry,
                                                d->hotkey_manager,
                                                d->path_manager,
                                                d->plugin_registry,
                                                d->query_engine,
                                                d->tray_icon,
                                                d->telemetry);
    hide();
    d->settings_window->bringToFront(plugin_id);
}

void Application::show(const QString &text)
{
    if (!text.isNull())
        d->frontend.setInput(text);
    d->frontend.setVisible(true);
}

void Application::hide() { d->frontend.setVisible(false); }

void Application::toggle() { d->frontend.setVisible(!d->frontend.isVisible()); }

namespace albert {

int ALBERT_EXPORT run(int argc, char **argv)
{
    if (qApp != nullptr)
        qFatal("Calling run more than once is not allowed.");

    QLoggingCategory::setFilterRules("*.debug=false");
    qInstallMessageHandler(messageHandler);


    // Parse command line (asap for fast cli commands)

    struct {
        QStringList plugin_dirs;
        bool autoload = true;
    } config;

    {
        auto opt_p = QCommandLineOption({"p", "plugin-dirs"},
                                        Application::tr("Set the plugin dirs to use. Comma separated."),
                                        Application::tr("directories"));
        auto opt_n = QCommandLineOption({"n", "no-autoload"},
                                        Application::tr("Do not implicitly load enabled plugins."));

        QCommandLineParser parser;
        parser.addOptions({opt_p, opt_n});
        parser.addPositionalArgument(Application::tr("command"),
                                     Application::tr("RPC command to send to the running instance."),
                                     Application::tr("[command [params...]]"));
        parser.addVersionOption();
        parser.addHelpOption();
        parser.setApplicationDescription(Application::tr("Launch Albert or control a running instance."));

        QCoreApplication qcoreapp(argc, argv);
        QCoreApplication::setApplicationName("albert");
        QCoreApplication::setApplicationVersion(ALBERT_VERSION_STRING);
        parser.process(qcoreapp);

        if (const auto args = parser.positionalArguments(); !args.isEmpty())
        {
            if (auto reply = RPCServer::sendMessage(
                    QJsonDocument(QJsonArray::fromStringList(args)).toJson(QJsonDocument::Compact)))
            {
                cout << reply->data() << endl;
                ::exit(EXIT_SUCCESS);
            }
            else
            {
                cout << reply.error().toStdString() << endl;
                ::exit(EXIT_FAILURE);
            }
        }

        config = {
            .plugin_dirs = parser.value(opt_p).split(',', Qt::SkipEmptyParts),
            .autoload    = !parser.isSet(opt_n),
        };
    }



    // Initialize Qt application

    QApplication qapp(argc, argv);
    QApplication::setApplicationName("albert");
    QApplication::setApplicationDisplayName("Albert");
    QApplication::setApplicationVersion(ALBERT_VERSION_STRING);
    QApplication::setWindowIcon(QIcon::fromTheme("albert"));
    QApplication::setQuitOnLastWindowClosed(false);
    for (const auto &line : report())
        DEBG << line;

    // Initialize app directories

    using namespace std::filesystem;
    for (const auto &path : { App::cacheLocation(), App::configLocation(), App::dataLocation() })
        try {
            create_directories(path);
            permissions(path, perms::owner_read | perms::owner_write | perms::owner_exec);
        } catch (...) {
            qFatal("Failed creating directory: %s", path.c_str());
        }


    // Initialize theme icon lookup

    {
        // QIcon::setThemeSearchPaths({":/icons"});  // implicitly set
        // See https://bugreports.qt.io/browse/QTBUG-140639
        QIcon::setFallbackThemeName("fallback");
        DEBG << "Theme search paths:" << QIcon::themeSearchPaths();
    }


    // Run app
    if (int return_value = Application::run(config.plugin_dirs, config.autoload);
        return_value != -1)
        return return_value;

    runDetachedProcess(qApp->arguments(), current_path().c_str());
    return EXIT_SUCCESS;
}

}
