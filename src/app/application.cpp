// Copyright (c) 2023-2025 Manuel Schneider

#include "application.h"
#include "extensionregistry.h"
#include "frontend.h"
#include "iconutil.h"
#include "logging.h"
#include "messagehandler.h"
#include "platform.h"
#include "plugininstance.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
#include "pluginswidget.h"
#include "qtpluginprovider.h"
#include "queryengine.h"
#include "querywidget.h"
#include "report.h"
#include "rpcserver.h"
#include "session.h"
#include "settingswindow.h"
#include "signalhandler.h"
#include "systemutil.h"
#include "telemetry.h"
#include "triggersqueryhandler.h"
#include "urlhandler.h"
#include <QByteArray>
#include <QCommandLineParser>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QHotkey>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLibraryInfo>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTranslator>
#include <iostream>
#include <QPluginLoader>
Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert")
using namespace albert::detail;
using namespace albert;
using namespace std;


namespace {
App *app_instance = nullptr;
static const char *STATE_LAST_USED_VERSION = "last_used_version";
static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";
static const char* CFG_SHOWTRAY = "showTray";
static const bool  DEF_SHOWTRAY = true;
static const char *CFG_HOTKEY = "hotkey";
static const char *DEF_HOTKEY = "Ctrl+Space";
static const char *CFG_ADDITIONAL_PATH_ENTRIES = "additional_path_entires";
}

// -------------------------------------------------------------------------------------------------

App::App()
{
    if (app_instance)
        qFatal("There can be only one app instance.");
    app_instance = this;
}

App::~App() { app_instance = nullptr; }

App &App::instance() { return *app_instance; }

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

    Private(Application &app, const QStringList &additional_plugin_paths, bool load_enabled);

    void initialize();
    void finalize();

    void initTrayIcon();
    void initPathVariable(const QSettings &);
    void initHotkey(const QSettings &);
    void initRPC();
    void loadAnyFrontend(const QSettings &);
    QString loadFrontend(albert::PluginLoader *);
    void notifyVersionChange();

public:

    Application &app;

    // As early as possible
    RPCServer rpc_server; // Check for other instances first
    SignalHandler unix_signal_handler;
    const QStringList original_path_entries;
    QStringList additional_path_entries;

    // Core
    albert::ExtensionRegistry extension_registry;
    PluginRegistry plugin_registry;
    QtPluginProvider plugin_provider;
    QueryEngine query_engine;
    Telemetry telemetry;

    // Weak, lazy or optional
    albert::PluginLoader *frontend_plugin{nullptr};
    albert::detail::Frontend *frontend{nullptr};
    std::unique_ptr<QHotkey> hotkey{nullptr};
    std::unique_ptr<QSystemTrayIcon> tray_icon{nullptr};
    std::unique_ptr<QMenu> tray_menu{nullptr};
    std::unique_ptr<Session> session{nullptr};
    QPointer<SettingsWindow> settings_window{nullptr};

    PluginQueryHandler plugin_query_handler;
    TriggersQueryHandler triggers_query_handler;

};


Application::Private::Private(Application &q, const QStringList &additional_plugin_paths, bool load_enabled):
    app(q),
    original_path_entries(qEnvironmentVariable("PATH").split(u':', Qt::SkipEmptyParts)),
    plugin_registry(extension_registry, load_enabled),
    plugin_provider(additional_plugin_paths),
    query_engine(extension_registry),
    telemetry(plugin_registry, extension_registry),
    plugin_query_handler(plugin_registry),
    triggers_query_handler(query_engine)
{}

void Application::Private::initialize()
{
    auto settings = App::settings();

    platform::initPlatform();

    // Install scheme handler
    QDesktopServices::setUrlHandler("albert", &app, "handleUrl");

    loadAnyFrontend(*settings);

    platform::initNativeWindow(frontend->winId());

    // Invalidate sessions on handler removal or visibility change
    auto reset_session = [this]{
        session.reset();
        if (frontend->isVisible())
            session = make_unique<Session>(query_engine, *frontend);
    };
    connect(frontend, &Frontend::visibleChanged, &app, reset_session);
    connect(&query_engine, &QueryEngine::queryHandlerRemoved, &app, reset_session);

    if (settings->value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool())
        initTrayIcon();

    initPathVariable(*settings);

    notifyVersionChange();

    initRPC(); // Also may trigger frontend

    initHotkey(*settings);  // Connect hotkey after! frontend has been loaded else segfaults

    extension_registry.registerExtension(&plugin_query_handler);
    extension_registry.registerExtension(&triggers_query_handler);

    // Load plugins not before loop is executing
    QTimer::singleShot(0, [this] { extension_registry.registerExtension(&plugin_provider); });
}

void Application::Private::finalize()
{
    QDesktopServices::unsetUrlHandler("albert");

    frontend->disconnect();
    query_engine.disconnect();

    if (hotkey)
    {
        hotkey.get()->disconnect();
        hotkey->setRegistered(false);
    }

    delete settings_window.get();
    session.reset();

    extension_registry.deregisterExtension(&plugin_provider);  // unloads plugins
    extension_registry.deregisterExtension(&triggers_query_handler);
    extension_registry.deregisterExtension(&plugin_query_handler);

    frontend_plugin->unload();
}

void Application::Private::initTrayIcon()
{
    // menu

    tray_menu = make_unique<QMenu>();

    auto *action = tray_menu->addAction(tr("Show/Hide"));
    connect(action, &QAction::triggered, [this] { app.toggle(); });

    action = tray_menu->addAction(tr("Settings"));
    connect(action, &QAction::triggered, [this] { app.showSettings(); });

    action = tray_menu->addAction(tr("Open website"));
    connect(action, &QAction::triggered, [] { open(QUrl("https://albertlauncher.github.io/")); });

    tray_menu->addSeparator();

    action = tray_menu->addAction(tr("Restart"));
    connect(action, &QAction::triggered, [this] { app.restart(); });

    action = tray_menu->addAction(tr("Quit"));
    connect(action, &QAction::triggered, [this] { app.quit(); });

    // icon

    auto icon = QIcon::fromTheme("albert-tray");
    icon.setIsMask(true);

    tray_icon = make_unique<QSystemTrayIcon>();
    tray_icon->setIcon(icon);
    tray_icon->setContextMenu(tray_menu.get());
    tray_icon->setVisible(true);

#ifndef Q_OS_MAC
    // Some systems open menus on right click, show albert on left trigger
    connect(tray_icon.get(), &QSystemTrayIcon::activated,
            &app, [this](QSystemTrayIcon::ActivationReason reason)
    {
        if( reason == QSystemTrayIcon::ActivationReason::Trigger)
            app.toggle();
    });
#endif
}

void Application::Private::initPathVariable(const QSettings &settings)
{
    additional_path_entries = settings.value(CFG_ADDITIONAL_PATH_ENTRIES).toStringList();
    auto effective_path_entries = QStringList() << additional_path_entries << original_path_entries;
    auto new_path = effective_path_entries.join(u':').toUtf8();
    qputenv("PATH", new_path);
    DEBG << "Effective PATH: " << new_path;
}

void Application::Private::initHotkey(const QSettings &settings)
{
    if (!QHotkey::isPlatformSupported())
    {
        INFO << "Hotkeys are not supported on this platform.";
        return;
    }

    auto s_hk = settings.value(CFG_HOTKEY, DEF_HOTKEY).toString();

    if (s_hk.isEmpty())
    {
        DEBG << "Hotkey explicitly unset.";
        return;
    }

    auto kc_hk = QKeySequence::fromString(s_hk)[0];

    if (auto hk = make_unique<QHotkey>(kc_hk);
        hk->setRegistered(true))
    {
        hotkey = ::move(hk);
        connect(hotkey.get(), &QHotkey::activated,
                frontend, [this]{ app.toggle(); });
        INFO << "Hotkey set to" << s_hk;
    }
    else
    {
        auto t = QT_TR_NOOP("Failed to set the hotkey '%1'");
        WARN << QString(t).arg(s_hk);
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                             tr(t).arg(QKeySequence(kc_hk)
                                       .toString(QKeySequence::NativeText)));
        app.showSettings();
    }
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

        else if (QUrl url(args[0]); url.isValid())
            for (const auto &arg : as_const(args))
                app.handleUrl(arg);

        else
        {
            WARN << "Invalid RPC message" << bytes;
        }

        return {};
    };

    rpc_server.setMessageHandler(messageHandler);
}

void Application::Private::loadAnyFrontend(const QSettings &settings)
{
    auto loaders = plugin_provider.frontendPlugins();
    const auto id = settings.value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();

    DEBG << QString("Try loading the configured frontend '%1'.").arg(id);

    if (auto it = ranges::find(loaders, id, [&](auto loader){ return loader->metadata().id; });
        it != loaders.end())
        if (auto err = loadFrontend(*it); err.isNull())
            return;
        else
        {
            WARN << QString("Loading configured frontend '%1' failed: %2.").arg(id, err);
            loaders.erase(it);
        }
    else
        WARN << QString("Configured frontend plugin '%1' does not exist.").arg(id);

    for (auto &loader : loaders)
    {
        WARN << QString("Try loading '%1'.").arg(loader->metadata().id);

        if (auto err = loadFrontend(loader);
            err.isNull())
        {
            INFO << QString("Using '%1' as fallback.").arg(loader->metadata().id);
            return;
        }
        else
            WARN << QString("Failed loading '%1'.").arg(loader->metadata().id);
    }

    qFatal("Could not load any frontend.");
}

QString Application::Private::loadFrontend(PluginLoader *loader)
{
    using enum Plugin::State;

    // Blocking load
    QEventLoop loop;
    connect(loader, &PluginLoader::finished,
            &loop, [&](QString info) { DEBG << info; loop.quit(); });
    loader->load();
    if (!loader->instance())  // sync cases
        loop.exec();

    if (frontend = dynamic_cast<Frontend*>(loader->instance());
        !frontend)
        return QString("Failed casting plugin instance to albert::Frontend: %1")
            .arg(loader->metadata().id);
    else
    {
        for (auto *ext : loader->instance()->extensions())
            extension_registry.registerExtension(ext);
        frontend_plugin = loader;
        return {};
    }
}

void Application::Private::notifyVersionChange()
{
    auto s = state();
    auto current_version = qApp->applicationVersion();
    auto last_used_version = s->value(STATE_LAST_USED_VERSION).toString();

    // First run
    if (last_used_version.isNull())
    {
        auto text = tr("This is the first time you've launched Albert. Albert is "
                       "plugin based. You have to enable some plugins you want to use.");

        QMessageBox::information(nullptr, qApp->applicationDisplayName(), text);

        app.showSettings();
    }
    else if (current_version.section('.', 0, 0) != last_used_version.section('.', 0, 0))
    {
        auto text = tr("You are now using Albert %1. The major version changed. "
                       "Some parts of the API might have changed. "
                       "Check the <a href=\"https://albertlauncher.github.io/news/\">news</a>."
                       ).arg(current_version);

        QMessageBox::information(nullptr, qApp->applicationDisplayName(), text);
    }

    if (last_used_version != current_version)
        s->setValue(STATE_LAST_USED_VERSION, current_version);
}

// -------------------------------------------------------------------------------------------------

Application::Application(const QStringList &additional_plugin_paths, bool load_enabled):
    d(make_unique<Private>(*this, additional_plugin_paths, load_enabled)) { }

Application::~Application() = default;

Application &Application::instance() { return static_cast<Application&>(App::instance()); }

void Application::initialize() { return d->initialize(); }

void Application::finalize() { return d->finalize(); }

void Application::handleUrl(const QUrl &url)
{
    DEBG << "Handle url" << url.toString();
    if (url.scheme() == qApp->applicationName())
    {
        if (url.authority().isEmpty())
        {
            // ?
        }
        else if (auto h = d->extension_registry.extension<UrlHandler>(url.authority()); h)
            h->handle(url);
        else
            WARN << "URL handler not available: " + url.authority().toLocal8Bit();
    }
    else
        WARN << "Invalid URL scheme" << url.scheme();
}

PluginRegistry &Application::pluginRegistry() { return d->plugin_registry; }

QueryEngine &Application::queryEngine() { return d->query_engine; }

Telemetry &Application::telemetry() { return d->telemetry; }

const ExtensionRegistry &Application::extensionRegistry() const { return d->extension_registry; }

void Application::showSettings(QString plugin_id)
{
    if (!d->settings_window)
        d->settings_window = new SettingsWindow(*this);
    hide();
    d->settings_window->bringToFront(plugin_id);
}

void Application::show(const QString &text)
{
    if (!text.isNull())
        d->frontend->setInput(text);
    d->frontend->setVisible(true);
}

void Application::hide() { d->frontend->setVisible(false); }

void Application::toggle() { d->frontend->setVisible(!d->frontend->isVisible()); }

Frontend *Application::frontend() { return d->frontend; }

QString Application::currentFrontend() { return d->frontend_plugin->metadata().name; }


QStringList Application::availableFrontends()
{
    QStringList ret;
    for (const auto *loader : d->plugin_provider.frontendPlugins())
        ret << loader->metadata().name;
    return ret;
}

void Application::setFrontend(uint i)
{
    auto fp = d->plugin_provider.frontendPlugins().at(i);
    settings()->setValue(CFG_FRONTEND_ID, fp->metadata().id);

    auto text = tr("Changing the frontend requires a restart. "
                   "Do you want to restart Albert?");

    if (QMessageBox::question(nullptr, qApp->applicationDisplayName(), text) == QMessageBox::Yes)
        restart();
}

bool Application::trayEnabled() const { return d->tray_icon.get(); }

void Application::setTrayEnabled(bool enable)
{
    if (enable && !trayEnabled())
        d->initTrayIcon();
    else if (!enable && trayEnabled()) {
        d->tray_icon.reset();
        d->tray_menu.reset();
    }
    else
        return;
    settings()->setValue(CFG_SHOWTRAY, enable);
}

const QStringList &Application::originalPathEntries() const { return d->original_path_entries; }

const QStringList &Application::additionalPathEntries() const { return d->additional_path_entries; }

void Application::setAdditionalPathEntries(const QStringList &entries)
{
    if (entries != d->additional_path_entries)
    {
        d->additional_path_entries = entries;
        settings()->setValue(CFG_ADDITIONAL_PATH_ENTRIES, entries);
    }
}

const QHotkey *Application::hotkey() const { return d->hotkey.get(); }

void Application::setHotkey(unique_ptr<QHotkey> hk)
{
    if (!hk)
    {
        d->hotkey.reset();
        settings()->setValue(CFG_HOTKEY, QString{});
    }
    else if (hk->isRegistered())
    {
        d->hotkey = ::move(hk);
        connect(d->hotkey.get(), &QHotkey::activated,
                d->frontend, [this]{ toggle(); });
        settings()->setValue(CFG_HOTKEY, d->hotkey->shortcut().toString());
    }
    else
        WARN << "Set unregistered hotkey. Ignoring.";
}

namespace albert {

int ALBERT_EXPORT run(int argc, char **argv)
{
    if (qApp != nullptr)
        qFatal("Calling run more than once is not allowed.");

    QLoggingCategory::setFilterRules("*.debug=false");
    qInstallMessageHandler(messageHandler);

    // Initialize Qt application

    QApplication qapp(argc, argv);
    QApplication::setApplicationName("albert");
    QApplication::setApplicationDisplayName("Albert");
    QApplication::setApplicationVersion(ALBERT_VERSION_STRING);
    QApplication::setWindowIcon(qIcon(makeThemeIcon("albert")));
    QApplication::setQuitOnLastWindowClosed(false);


    // Parse command line (asap for fast cli commands)

    struct {
        QStringList plugin_dirs;
        bool autoload;
    } config;

    {
        auto opt_p = QCommandLineOption({"p", "plugin-dirs"},
                                        Application::tr("Set the plugin dirs to use. Comma separated."),
                                        Application::tr("directories"));
        auto opt_r = QCommandLineOption({"r", "report"},
                                        Application::tr("Print report and quit."));
        auto opt_n = QCommandLineOption({"n", "no-autoload"},
                                        Application::tr("Do not implicitly load enabled plugins."));

        QCommandLineParser parser;
        parser.addOptions({opt_p, opt_r, opt_n});
        parser.addPositionalArgument(Application::tr("command"),
                                     Application::tr("RPC command to send to the running instance."),
                                     Application::tr("[command [params...]]"));
        parser.addVersionOption();
        parser.addHelpOption();
        parser.setApplicationDescription(Application::tr("Launch Albert or control a running instance."));
        parser.process(qapp);

        // TODO If not running? Continue and use? Makes sense for albert show but not for URLs.
        if (const auto args = parser.positionalArguments(); !args.isEmpty())
            try {
                QJsonDocument d(QJsonArray::fromStringList(args));
                auto bytes = d.toJson(QJsonDocument::Compact);
                bytes = RPCServer::sendMessage(bytes);
                cout << bytes.data() << endl;
                return EXIT_SUCCESS;
            } catch (const exception &e) {
                cout << e.what() << endl;
                return EXIT_FAILURE;
            }

        if (parser.isSet(opt_r)) {
            for (const auto &line : report())
                std::cout << line.toStdString() << std::endl;
            ::exit(EXIT_SUCCESS);
        } else
            for (const auto &line : report())
                DEBG << line;

        config = {
            .plugin_dirs = parser.value(opt_p).split(',', Qt::SkipEmptyParts),
            .autoload    = !parser.isSet(opt_n),
        };
    }


    // Initialize app directories

    for (const auto &path : { App::cacheLocation(), App::configLocation(), App::dataLocation() })
        try {
            filesystem::create_directories(path);
            QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        } catch (...) {
            qFatal("Failed creating directory: %s", path.c_str());
        }


    // Load translators

    {
        DEBG << "Loading translations";

        auto *t = new QTranslator(&qapp);
        if (t->load(QLocale(), "qtbase", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        {
            DEBG << " -" << t->filePath();
            qapp.installTranslator(t);
        }
        else
            delete t;

        t = new QTranslator(&qapp);
        if (t->load(QLocale(), qapp.applicationName(), "_", ":/i18n"))
        {
            DEBG << " -" << t->filePath();
            qapp.installTranslator(t);
        }
        else
            delete t;
    }


    // Initialize theme icon lookup

    {
        // QIcon::setThemeSearchPaths({":/icons"});  // implicitly set
        // See https://bugreports.qt.io/browse/QTBUG-140639
        QIcon::setFallbackThemeName("fallback");
        DEBG << "Theme search paths:" << QIcon::themeSearchPaths();
    }


    // Run app

    Application app(config.plugin_dirs, config.autoload);
    app.initialize();
    int return_value = qapp.exec();
    app.finalize();

    if (return_value == -1 && runDetachedProcess(qApp->arguments(), QDir::currentPath()))
        return_value = EXIT_SUCCESS;

    return return_value;
}

}
