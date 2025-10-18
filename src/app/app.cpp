// Copyright (c) 2023-2025 Manuel Schneider

#include "albert.h"
#include "app.h"
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
static App * app_instance{nullptr};
static const char *STATE_LAST_USED_VERSION = "last_used_version";
static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";
static const char* CFG_SHOWTRAY = "showTray";
static const bool  DEF_SHOWTRAY = true;
static const char *CFG_HOTKEY = "hotkey";
static const char *DEF_HOTKEY = "Ctrl+Space";
static const char *CFG_ADDITIONAL_PATH_ENTRIES = "additional_path_entires";
}


class App::Private
{
public:

    Private(const QStringList &additional_plugin_paths, bool load_enabled);

    void initialize();
    void finalize();

    void initTrayIcon();
    void initPathVariable(const QSettings &settings);
    void initHotkey();
    void initRPC();
    void loadAnyFrontend();
    QString loadFrontend(albert::PluginLoader *loader);
    void notifyVersionChange();

public:

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


App::Private::Private(const QStringList &additional_plugin_paths, bool load_enabled):
    original_path_entries(qEnvironmentVariable("PATH").split(u':', Qt::SkipEmptyParts)),
    plugin_registry(extension_registry, load_enabled),
    plugin_provider(additional_plugin_paths),
    query_engine(extension_registry),
    telemetry(extension_registry),
    plugin_query_handler(plugin_registry),
    triggers_query_handler(query_engine)
{}

void App::Private::initialize()
{
    platform::initPlatform();

    // Install scheme handler
    QDesktopServices::setUrlHandler("albert", app_instance, "handleUrl");

    loadAnyFrontend();

    platform::initNativeWindow(frontend->winId());

    // Invalidate sessions on handler removal or visibility change
    auto reset_session = [this]{
        session.reset();
        if (frontend->isVisible())
            session = make_unique<Session>(query_engine, *frontend);
    };
    connect(frontend, &Frontend::visibleChanged, app_instance, reset_session);
    connect(&query_engine, &QueryEngine::handlerRemoved, app_instance, reset_session);

    auto settings = albert::settings();

    if (settings->value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool())
        initTrayIcon();

    initPathVariable(*settings);

    notifyVersionChange();

    initRPC(); // Also may trigger frontend

    initHotkey();  // Connect hotkey after! frontend has been loaded else segfaults

    extension_registry.registerExtension(&plugin_query_handler);
    extension_registry.registerExtension(&triggers_query_handler);

    // Load plugins not before loop is executing
    QTimer::singleShot(0, [this] { extension_registry.registerExtension(&plugin_provider); });
}

void App::Private::finalize()
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

void App::Private::initTrayIcon()
{
    // menu

    tray_menu = make_unique<QMenu>();

    auto *action = tray_menu->addAction(tr("Show/Hide"));
    connect(action, &QAction::triggered, [] { App::instance()->toggle(); });

    action = tray_menu->addAction(tr("Settings"));
    connect(action, &QAction::triggered, [] { App::instance()->showSettings(); });

    action = tray_menu->addAction(tr("Open website"));
    connect(action, &QAction::triggered, [] { open(QUrl("https://albertlauncher.github.io/")); });

    tray_menu->addSeparator();

    action = tray_menu->addAction(tr("Restart"));
    connect(action, &QAction::triggered, [] { albert::restart(); });

    action = tray_menu->addAction(tr("Quit"));
    connect(action, &QAction::triggered, [] { albert::quit(); });

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
            [](QSystemTrayIcon::ActivationReason reason)
    {
        if( reason == QSystemTrayIcon::ActivationReason::Trigger)
            App::instance()->toggle();
    });
#endif
}

void App::Private::initPathVariable(const QSettings &settings)
{
    additional_path_entries = settings.value(CFG_ADDITIONAL_PATH_ENTRIES).toStringList();
    auto effective_path_entries = QStringList() << additional_path_entries << original_path_entries;
    auto new_path = effective_path_entries.join(u':').toUtf8();
    qputenv("PATH", new_path);
    DEBG << "Effective PATH: " << new_path;
}

void App::Private::initHotkey()
{
    if (!QHotkey::isPlatformSupported())
    {
        INFO << "Hotkeys are not supported on this platform.";
        return;
    }

    auto s_hk = settings()->value(CFG_HOTKEY, DEF_HOTKEY).toString();

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
                frontend, []{ App::instance()->toggle(); });
        INFO << "Hotkey set to" << s_hk;
    }
    else
    {
        auto t = QT_TR_NOOP("Failed to set the hotkey '%1'");
        WARN << QString(t).arg(s_hk);
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                             tr(t).arg(QKeySequence(kc_hk)
                                       .toString(QKeySequence::NativeText)));
        App::instance()->showSettings();
    }
}

void App::Private::initRPC()
{
    auto messageHandler = [](const QByteArray bytes) -> QByteArray
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
                App::instance()->show(args[1]);

            else // if (args.size() == 1)
                App::instance()->show();
        }

        else if (args[0] == "hide")

            if (args.size() == 1)
                App::instance()->hide();
            else
                return "'hide' expects no arguments.";

        else if (args[0] == "toggle")

            if (args.size() == 1)
                App::instance()->toggle();
            else
                return "'toggle' expects no arguments.";

        else if (args[0] == "settings")
        {
            if (args.size() > 2)
                return "'settings' expects zero or one argument.";

            else if (args.size() == 2)
                App::instance()->showSettings(args[1]);

            else // if (args.size() == 1)
                App::instance()->showSettings();
        }

        else if (args[0] == "restart")

            if (args.size() == 1)
                App::instance()->restart();
            else
                return "'restart' expects no arguments.";

        else if (args[0] == "quit")

            if (args.size() == 1)
                App::instance()->quit();
            else
                return "'quit' expects no arguments.";

        else if (args[0] == "report")

            if (args.size() == 1)
                return report().join('\n').toLocal8Bit();
            else
                return "'report' expects no arguments.";

        else if (QUrl url(args[0]); url.isValid())
            for (const auto &arg : as_const(args))
                app_instance->handleUrl(arg);

        else
        {
            WARN << "Invalid RPC message" << bytes;
        }

        return {};
    };

    rpc_server.setMessageHandler(messageHandler);
}

void App::Private::loadAnyFrontend()
{
    auto loaders = plugin_provider.frontendPlugins();
    const auto id = settings()->value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();

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

QString App::Private::loadFrontend(PluginLoader *loader)
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

void App::Private::notifyVersionChange()
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

        App::instance()->showSettings();
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

App::App(const QStringList &additional_plugin_paths, bool load_enabled)
{
    if (app_instance)
        qFatal("No multiple app instances allowed");

    app_instance = this; // must be valid before Private is constructed
    d = make_unique<Private>(additional_plugin_paths, load_enabled);
}

App::~App() = default;

App *App::instance() { return app_instance; }

void App::initialize() { return d->initialize(); }

void App::finalize() { return d->finalize(); }

void App::handleUrl(const QUrl &url)
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

PluginRegistry &App::pluginRegistry() { return d->plugin_registry; }

QueryEngine &App::queryEngine() { return d->query_engine; }

Telemetry &App::telemetry() { return d->telemetry; }

void App::showSettings(QString plugin_id)
{
    if (!d->settings_window)
        d->settings_window = new SettingsWindow(*this);
    hide();
    d->settings_window->bringToFront(plugin_id);
}

void App::show(const QString &text)
{
    if (!text.isNull())
        d->frontend->setInput(text);
    d->frontend->setVisible(true);
}

void App::hide() { d->frontend->setVisible(false); }

void App::toggle() { d->frontend->setVisible(!d->frontend->isVisible()); }

void App::restart()
{
    QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1));
}

void App::quit() { QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection); }

Frontend *App::frontend() { return d->frontend; }

QString App::currentFrontend() { return d->frontend_plugin->metadata().name; }

ExtensionRegistry &App::extensionRegistry() { return d->extension_registry; }

QStringList App::availableFrontends()
{
    QStringList ret;
    for (const auto *loader : d->plugin_provider.frontendPlugins())
        ret << loader->metadata().name;
    return ret;
}

void App::setFrontend(uint i)
{
    auto fp = d->plugin_provider.frontendPlugins().at(i);
    settings()->setValue(CFG_FRONTEND_ID, fp->metadata().id);

    auto text = tr("Changing the frontend requires a restart. "
                   "Do you want to restart Albert?");

    if (QMessageBox::question(nullptr, qApp->applicationDisplayName(), text) == QMessageBox::Yes)
        restart();
}

bool App::trayEnabled() const { return d->tray_icon.get(); }

void App::setTrayEnabled(bool enable)
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

const QStringList &App::originalPathEntries() const { return d->original_path_entries; }

const QStringList &App::additionalPathEntries() const { return d->additional_path_entries; }

void App::setAdditionalPathEntries(const QStringList &entries)
{
    if (entries != d->additional_path_entries)
    {
        d->additional_path_entries = entries;
        settings()->setValue(CFG_ADDITIONAL_PATH_ENTRIES, entries);
    }
}

const QHotkey *App::hotkey() const { return d->hotkey.get(); }

void App::setHotkey(unique_ptr<QHotkey> hk)
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
                d->frontend, []{ App::instance()->toggle(); });
        settings()->setValue(CFG_HOTKEY, d->hotkey->shortcut().toString());
    }
    else
        WARN << "Set unregistered hotkey. Ignoring.";
}


namespace albert
{

int ALBERT_EXPORT run(int argc, char **argv)
{
    if (qApp != nullptr)
        qFatal("Calling main twice is not allowed.");

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
                                        App::tr("Set the plugin dirs to use. Comma separated."),
                                        App::tr("directories"));
        auto opt_r = QCommandLineOption({"r", "report"},
                                        App::tr("Print report and quit."));
        auto opt_n = QCommandLineOption({"n", "no-autoload"},
                                        App::tr("Do not implicitly load enabled plugins."));

        QCommandLineParser parser;
        parser.addOptions({opt_p, opt_r, opt_n});
        parser.addPositionalArgument(App::tr("command"),
                                     App::tr("RPC command to send to the running instance."),
                                     App::tr("[command [params...]]"));
        parser.addVersionOption();
        parser.addHelpOption();
        parser.setApplicationDescription(App::tr("Launch Albert or control a running instance."));
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

    for (const auto &path : { cacheLocation(), configLocation(), dataLocation() })
        try {
            filesystem::create_directories(path);
            QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        } catch (...) {
            qFatal("Failed creating directory: %s", path.c_str());
        }


    // Section for ports

    {
        // Move old config file to new location TODO: Remove from 0.26 on

        {
            auto conf_loc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
            auto old_conf_loc = QDir(conf_loc).filePath("albert.conf");
            QFile config_file(old_conf_loc);
            if (config_file.exists())
            {
                auto new_conf_loc = QDir(configLocation()).filePath("config");
                if (config_file.rename(new_conf_loc))
                    INFO << "Config file successfully moved to new location.";
                else
                    qFatal("Failed to move config file to new location. "
                           "Please move the file at %s to %s manually.",
                           old_conf_loc.toUtf8().data(), new_conf_loc.toUtf8().data());
            }
        }

        // Merge settings sections of applications plugins

        {
            auto s = settings();
            auto groups = s->childGroups();

            for (const char *old_group : { "applications_macos", "applications_xdg"})
            {
                if (groups.contains(old_group))
                {
                    s->beginGroup(old_group);
                    auto child_keys = s->childKeys();
                    s->endGroup();

                    for (const QString &child_key : as_const(child_keys))
                    {
                        auto old_key = QString("%1/%2").arg(old_group, child_key);

                        s->setValue(QString("applications/%1").arg(child_key),
                                    s->value(old_key));

                        s->remove(old_key);
                    }
                }
            }
        }

        // Move state file from cache to data dir

        {
            using namespace std::filesystem;
            const auto old_path = cacheLocation() / "state";
            const auto new_path = dataLocation() / "state";

            if(!exists(new_path.parent_path()))
                create_directories(new_path.parent_path());

            if (exists(old_path))
            {
                if (exists(new_path))
                    remove(old_path);
                else
                    rename(old_path, new_path);
            }
        }
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
    }


    // Run app

    try {
        app_instance = new App(config.plugin_dirs, config.autoload);
        app_instance->initialize();
        int return_value = qapp.exec();
        app_instance->finalize();
        delete app_instance;

        if (return_value == -1 && runDetachedProcess(qApp->arguments(), QDir::currentPath()))
            return_value = EXIT_SUCCESS;

        INFO << "Bye.";
        return return_value;
    } catch (const std::exception &e) {
        CRIT << "Uncaught exception in main: " << e.what();
        return EXIT_FAILURE;
    } catch (...) {
        CRIT << "Uncaught unknown exception in main. Exiting.";
        return EXIT_FAILURE;
    }
}

}
