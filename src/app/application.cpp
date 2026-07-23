// Copyright (c) 2023-2025 Manuel Schneider

#include "app.h"
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
#include <memory>
Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert")
using namespace Qt::StringLiterals;
using namespace albert::detail;
using namespace albert;
using namespace std::filesystem;
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



// -------------------------------------------------------------------------------------------------

class Application final : public albert::App
{
public:

    struct Config
    {
        QStringList additional_plugin_paths;
        bool load_enabled = true;
    };

    Application(const Config &config);
    ~Application();

    void show(const QString &text = {}) override;
    void hide();
    void toggle();

    void showSettings(QString plugin_id = {}) override;

    bool localizationEnabled() const override;

    const std::map<QString, albert::Extension *> &extensions() const override;

    const std::filesystem::path &cacheLocation() override;
    const std::filesystem::path &configLocation() override;
    const std::filesystem::path &dataLocation() override;

    std::unique_ptr<QSettings> settings() override;
    std::unique_ptr<QSettings> state() override;

    void initRPC();

    QString loadFrontend(albert::PluginLoader *);
    void notifyVersionChange(QSettings &state);

public:

    const std::filesystem::path cache_location;
    const std::filesystem::path config_location;
    const std::filesystem::path data_location;

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

static filesystem::path initBaseDirectory(QStandardPaths::StandardLocation location)
{
    path path = QStandardPaths::writableLocation(location).toStdString();
    create_directories(path);
    permissions(path, perms::owner_read | perms::owner_write | perms::owner_exec);
    return path;
}

Application::Application(const Config &cfg):
    cache_location(initBaseDirectory(QStandardPaths::CacheLocation)),
    config_location(initBaseDirectory(QStandardPaths::AppConfigLocation)),
    data_location(initBaseDirectory(QStandardPaths::AppDataLocation)),
    path_manager(*settings()),
    localization(*settings()),
    plugin_registry(extension_registry, cfg.load_enabled),
    plugin_provider(cfg.additional_plugin_paths),
    frontend_registry(*settings(), plugin_provider),
    frontend(frontend_registry.frontend()),
    query_engine(extension_registry),
    telemetry(plugin_registry, extension_registry),
    tray_icon(*settings(), frontend),
    hotkey_manager(*settings()),
    plugin_query_handler(plugin_registry),
    triggers_query_handler(query_engine)
{
    auto settings = this->settings();
    auto state = this->state();

    platform::initPlatform();
    platform::initNativeWindow(frontend.winId());

    for (auto *ext : frontend.extensions())
        extension_registry.registerExtension(ext);
    extension_registry.registerExtension(&plugin_query_handler);
    extension_registry.registerExtension(&triggers_query_handler);
    // Load plugins not before loop is executing
    QTimer::singleShot(0, [this] { extension_registry.registerExtension(&plugin_provider); });

    connect(&frontend, &Frontend::visibleChanged,
            this, [this]{
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
            this, reset_session);

    connect(&query_engine, &QueryEngine::queryHandlerRemoved,
            this, reset_session, Qt::QueuedConnection);

    connect(&hotkey_manager, &HotkeyManager::activated,
            this, &Application::toggle);

    connect(&extension_registry, &ExtensionRegistry::added,
            this, &Application::added);

    connect(&extension_registry, &ExtensionRegistry::removed,
            this, &Application::removed);

    initRPC(); // Also may trigger frontend

    notifyVersionChange(*state);
}

Application::~Application()
{
    delete settings_window.get();
    session.reset();

    extension_registry.deregisterExtension(&plugin_provider);  // unloads plugins
    extension_registry.deregisterExtension(&triggers_query_handler);
    extension_registry.deregisterExtension(&plugin_query_handler);
}

const filesystem::path &Application::cacheLocation() { return cache_location; }

const filesystem::path &Application::configLocation() { return config_location; }

const filesystem::path &Application::dataLocation() { return data_location; }

unique_ptr<QSettings> Application::settings()
{ return make_unique<QSettings>(toQString(config_location / "config"), QSettings::IniFormat); }

unique_ptr<QSettings> Application::state()
{ return make_unique<QSettings>(toQString(data_location / "state"), QSettings::IniFormat); }

const map<QString, Extension *> &Application::extensions() const
{ return extension_registry.extensions(); }

void Application::showSettings(QString plugin_id)
{
    if (!settings_window)
        settings_window = new SettingsWindow(frontend_registry,
                                             hotkey_manager,
                                             path_manager,
                                             plugin_registry,
                                             query_engine,
                                             tray_icon,
                                             telemetry);
    hide();
    settings_window->bringToFront(plugin_id);
}

bool Application::localizationEnabled() const { return localization.isEnabled(); }

void Application::show(const QString &text)
{
    if (!text.isNull())
        frontend.setInput(text);
    frontend.setVisible(true);
}

void Application::hide() { frontend.setVisible(false); }

void Application::toggle() { frontend.setVisible(!frontend.isVisible()); }

void Application::initRPC()
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
                show(args[1]);

            else // if (args.size() == 1)
                show();
        }

        else if (args[0] == "hide")

            if (args.size() == 1)
                hide();
            else
                return "'hide' expects no arguments.";

        else if (args[0] == "toggle")

            if (args.size() == 1)
                toggle();
            else
                return "'toggle' expects no arguments.";

        else if (args[0] == "settings")
        {
            if (args.size() > 2)
                return "'settings' expects zero or one argument.";

            else if (args.size() == 2)
                showSettings(args[1]);

            else // if (args.size() == 1)
                showSettings();
        }

        else if (args[0] == "restart")

            if (args.size() == 1)
                restart();
            else
                return "'restart' expects no arguments.";

        else if (args[0] == "quit")

            if (args.size() == 1)
                quit();
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

void Application::notifyVersionChange(QSettings &state)
{
    auto current_version = qApp->applicationVersion();
    auto last_used_version = state.value(STATE_LAST_USED_VERSION).toString();

    // First run
    if (last_used_version.isNull())
    {
        auto text = tr("This is the first time you've launched Albert. Albert is "
                       "plugin based. You have to enable some plugins you want to use.");

        information(text);

        QTimer::singleShot(0, this, [this]{ showSettings(); });
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

namespace albert::detail {
int ALBERT_EXPORT run(int argc, char **argv)
{
    if (qApp != nullptr)
        qFatal("Calling run more than once is not allowed.");

    Application::Config config;
    {
        QCoreApplication qcoreapp(argc, argv);
        QCoreApplication::setApplicationName("albert");
        QCoreApplication::setApplicationVersion(ALBERT_VERSION_STRING);

        auto opt_p = QCommandLineOption(
            {"p", "plugin-dirs"},
            Application::tr("Set the plugin dirs to use. Comma separated."),
            Application::tr("directories"));

        auto opt_n = QCommandLineOption(
            {"n", "no-autoload"},
            Application::tr("Do not implicitly load enabled plugins."));

        QCommandLineParser parser;
        parser.addOptions({opt_p, opt_n});
        parser.addPositionalArgument(Application::tr("command"),
                                     Application::tr("RPC command to send to the running instance."),
                                     Application::tr("[command [params...]]"));
        parser.addVersionOption();
        parser.addHelpOption();
        parser.setApplicationDescription(Application::tr("Launch Albert or control a running instance."));
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

        config.additional_plugin_paths = parser.value(opt_p).split(',', Qt::SkipEmptyParts);
        config.load_enabled            = !parser.isSet(opt_n);
    }


    // Initialize Qt application

    QLoggingCategory::setFilterRules("*.debug=false");
    qInstallMessageHandler(messageHandler);

    QApplication qapp(argc, argv);
    QApplication::setApplicationName("albert");
    QApplication::setApplicationDisplayName("Albert");
    QApplication::setApplicationVersion(ALBERT_VERSION_STRING);
    QApplication::setWindowIcon(QIcon::fromTheme("albert"));
    QApplication::setQuitOnLastWindowClosed(false);

    // Initialize theme icon lookup

    {
        // QIcon::setThemeSearchPaths({":/icons"});  // implicitly set
        // See https://bugreports.qt.io/browse/QTBUG-140639
        QIcon::setFallbackThemeName("fallback");
        DEBG << "Theme search paths:" << QIcon::themeSearchPaths();
    }

    for (const auto &line : report())
        DEBG << line;

    int return_value = [&] {
        Application app(config);
        return qApp->exec();
    }();

    if (return_value != -1)
        return return_value;

    runDetachedProcess(qApp->arguments(), current_path().c_str());
    return EXIT_SUCCESS;
}

}
