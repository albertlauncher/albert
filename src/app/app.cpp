// Copyright (c) 2023-2024 Manuel Schneider

#include "app.h"
#include "appqueryhandler.h"
#include "extensionregistry.h"
#include "frontend.h"
#include "iconprovider.h"
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
#include "telemetry.h"
#include "terminalprovider.h"
#include "triggersqueryhandler.h"
#include "util.h"
#include <QCommandLineParser>
#include <QHotkey>
#include <QLibraryInfo>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTranslator>
#ifdef Q_OS_UNIX
#include "platform/unix/unixsignalhandler.h"
#endif
Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert")
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
static const char *CFG_TELEMETRY = "telemetry";
}


class App::Private
{
public:

    Private(const QStringList &additional_plugin_paths, bool load_enabled);

    void initialize();
    void finalize();

    void initTrayIcon();
    void initTelemetry();
    void initHotkey();
    void initPRC();
    void loadAnyFrontend();
    QString loadFrontend(albert::PluginLoader *loader);
    void notifyVersionChange();

public:

    // As early as possible
    RPCServer rpc_server; // Check for other instances first
#ifdef Q_OS_UNIX
    UnixSignalHandler unix_signal_handler;
#endif

    // Core
    albert::ExtensionRegistry extension_registry;
    PluginRegistry plugin_registry;
    QtPluginProvider plugin_provider;
    QueryEngine query_engine;
    TerminalProvider terminal_provider;

    // Weak, lazy or optional
    albert::PluginLoader *frontend_plugin{nullptr};
    albert::Frontend *frontend{nullptr};
    std::unique_ptr<QHotkey> hotkey{nullptr};
    std::unique_ptr<Telemetry> telemetry{nullptr};
    std::unique_ptr<QSystemTrayIcon> tray_icon{nullptr};
    std::unique_ptr<QMenu> tray_menu{nullptr};
    std::unique_ptr<Session> session{nullptr};
    QPointer<SettingsWindow> settings_window{nullptr};

    AppQueryHandler app_query_handler;
    PluginQueryHandler plugin_query_handler;
    TriggersQueryHandler triggers_query_handler;
};


App::Private::Private(const QStringList &additional_plugin_paths, bool load_enabled):
    plugin_registry(extension_registry, load_enabled),
    plugin_provider(additional_plugin_paths),
    query_engine(extension_registry),
    plugin_query_handler(plugin_registry),
    triggers_query_handler(query_engine) {}

void App::Private::initialize()
{
    platform::initPlatform();

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

    if (settings()->value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool())
        initTrayIcon();

    notifyVersionChange();

    initTelemetry();

    initPRC(); // Also may trigger frontend

    initHotkey();  // Connect hotkey after! frontend has been loaded else segfaults

    extension_registry.registerExtension(&app_query_handler);
    extension_registry.registerExtension(&plugin_query_handler);
    extension_registry.registerExtension(&triggers_query_handler);
    extension_registry.registerExtension(&plugin_provider);  // loads plugins
}

void App::Private::finalize()
{
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
    extension_registry.deregisterExtension(&app_query_handler);

    try {
        frontend_plugin->unload();
    } catch (const exception &e) {
        WARN << e.what();
    }
}

void App::Private::initTrayIcon()
{
    // menu

    tray_menu = make_unique<QMenu>();

    auto *action = tray_menu->addAction(tr("Show/Hide"));
    connect(action, &QAction::triggered,
            []{ App::instance()->toggle(); });

    action = tray_menu->addAction(tr("Settings"));
    connect(action, &QAction::triggered,
            []{ App::instance()->showSettings(); });

    action = tray_menu->addAction(tr("Open website"));
    connect(action, &QAction::triggered,
            []{ albert::openWebsite(); });

    tray_menu->addSeparator();

    action = tray_menu->addAction(tr("Restart"));
    connect(action, &QAction::triggered,
            []{ App::instance()->restart(); });

    action = tray_menu->addAction(tr("Quit"));
    connect(action, &QAction::triggered,
            []{ App::instance()->quit(); });

    // icon

    auto icon = albert::iconFromUrls({"xdg:albert-tray", "xdg:albert", ":app_tray_icon"});
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

void App::Private::initTelemetry()
{
    if (auto s = settings(); !s->contains(CFG_TELEMETRY))
    {
        auto text = tr("Albert collects anonymous data to enhance user experience. "
                       "You can review the data to be sent in the details. Opt in?");

        QMessageBox mb(QMessageBox::Question, qApp->applicationDisplayName(),
                       text, QMessageBox::No|QMessageBox::Yes);

        mb.setDefaultButton(QMessageBox::Yes);
        mb.setDetailedText(telemetry->buildReportString());
        s->setValue(CFG_TELEMETRY, mb.exec() == QMessageBox::Yes);
    }
}

void App::Private::initHotkey()
{
    if (!QHotkey::isPlatformSupported())
    {
        INFO << "Hotkeys are not supported on this platform.";
        return;
    }

    auto s_hk = settings()->value(CFG_HOTKEY, DEF_HOTKEY).toString();
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

void App::Private::initPRC()
{
    std::map<QString, RPCServer::RPC> rpc =
    {
        {"show", [](const QString& t){
            App::instance()->show(t);
            return "Albert set visible.";
        }},
        {"hide", [](const QString&){
            App::instance()->hide();
            return "Albert set hidden.";
        }},
        {"toggle", [](const QString&){
            App::instance()->toggle();
            return "Albert visibility toggled.";
        }},
        {"settings", [](const QString& t){
            App::instance()->showSettings(t);
            return "Settings opened,";
        }},
        {"restart", [](const QString&){
            App::instance()->restart();
            return "Triggered restart.";
        }},
        {"quit", [](const QString&){
            App::instance()->quit();
            return "Triggered quit.";
        }},
        {"report", [](const QString&){
            return report().join('\n');
        }}
    };

    rpc_server.setPRC(::move(rpc));
}

void App::Private::loadAnyFrontend()
{
    auto frontend_plugins = plugin_provider.frontendPlugins();

    auto cfg_frontend = settings()->value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();
    DEBG << QString("Try loading the configured frontend '%1'.").arg(cfg_frontend);
    if (auto it = find_if(frontend_plugins.begin(), frontend_plugins.end(),
                          [&](const PluginLoader *loader){ return cfg_frontend == loader->metaData().id; });
        it != frontend_plugins.end())
        if (auto err = loadFrontend(*it); err.isNull())
            return;
        else {
            WARN << QString("Loading configured frontend plugin '%1' failed: %2.").arg(cfg_frontend, err);
            frontend_plugins.erase(it);
        }
    else
        WARN << QString("Configured frontend plugin '%1' does not exist.").arg(cfg_frontend);

    for (auto &loader : frontend_plugins){
        DEBG << QString("Try loading frontend plugin '%1'.").arg(loader->metaData().id);;
        if (auto err = loadFrontend(loader); err.isNull()){
            INFO << QString("Using '%1' as fallback.").arg(loader->metaData().id);
            return;
        } else
            WARN << QString("Failed loading frontend plugin '%1'.").arg(loader->metaData().id);
    }

    qFatal("Could not load any frontend.");
}

QString App::Private::loadFrontend(PluginLoader *loader)
{
    try {
        PluginRegistry::staticDI.loader = loader;
        loader->load();

        plugin_registry.staticDI.loader = loader;
        auto * inst = loader->createInstance();
        if (!inst)
            return "Plugin loader returned null instance";

        frontend = dynamic_cast<Frontend*>(loader->createInstance());
        if (!frontend)
            return QString("Failed casting Plugin instance to albert::Frontend: %1").arg(loader->metaData().id);

        frontend_plugin = loader;

        return {};
    } catch (const exception &e) {
        return QString::fromStdString(e.what());
    } catch (...) {
        return "Unknown exception";
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
    else if (current_version.section('.', 1, 1) != last_used_version.section('.', 1, 1) )  // FIXME in first major version
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

TerminalProvider &App::terminal() { return d->terminal_provider; }

PluginRegistry &App::pluginRegistry() { return d->plugin_registry; }

QueryEngine &App::queryEngine() { return d->query_engine; }

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

void App::hide()
{
    d->frontend->setVisible(false);
}

void App::toggle()
{
    d->frontend->setVisible(!d->frontend->isVisible());
}

void App::restart()
{
    QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1));
}

void App::quit()
{
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

Frontend *App::frontend() { return d->frontend; }

QString App::currentFrontend() { return d->frontend_plugin->metaData().name; }

QStringList App::availableFrontends()
{
    QStringList ret;
    for (const auto *loader : d->plugin_provider.frontendPlugins())
        ret << loader->metaData().name;
    return ret;
}

void App::setFrontend(uint i)
{
    auto fp = d->plugin_provider.frontendPlugins().at(i);
    settings()->setValue(CFG_FRONTEND_ID, fp->metaData().id);

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
bool App::telemetryEnabled() const { return d->telemetry.get(); }

void App::setTelemetryEnabled(bool enable)
{
    if (enable && !telemetryEnabled())
        d->telemetry = make_unique<Telemetry>();
    else if (!enable && telemetryEnabled())
        d->telemetry.reset();
    else
        return;
    settings()->setValue(CFG_TELEMETRY, enable);
}

const QHotkey *App::hotkey() const { return d->hotkey.get(); }

void App::setHotkey(unique_ptr<QHotkey> hk)
{
    if (!hk)
    {
        d->hotkey.reset();
        settings()->remove(CFG_HOTKEY);
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


    // Put /usr/local/bin hardcoded to env… why?

    // {
    //     auto usr_local_bin = QStringLiteral("/usr/local/bin");
    //     auto PATHS = QString(qgetenv("PATH")).split(':');
    //     if (!PATHS.contains(usr_local_bin))
    //         PATHS.prepend(usr_local_bin);
    //     auto PATH = PATHS.join(':').toLocal8Bit();
    //     qputenv("PATH", PATH);
    // }


    // Set locale from env vars (for macos debug builds)

    // if (const char *key = "LANGUAGE"; qEnvironmentVariableIsSet(key))
    //     QLocale::setDefault(QLocale(qEnvironmentVariable(key)));
    // else if (key = "LANG"; qEnvironmentVariableIsSet(key))
    //     QLocale::setDefault(QLocale(qEnvironmentVariable(key)));


    // Initialize Qt application

    QApplication qapp(argc, argv);
    QApplication::setApplicationName("albert");
    QApplication::setApplicationDisplayName("Albert");
    QApplication::setApplicationVersion(ALBERT_VERSION_STRING);
    QApplication::setWindowIcon(iconFromUrls({"xdg:albert", "qrc:app_icon"}));
    QApplication::setQuitOnLastWindowClosed(false);


    // Initialize app directories

    for (const auto &path : { cacheLocation(), configLocation(), dataLocation() })
    {
        if (!QDir(path).mkpath("."))
            qFatal("Failed creating config dir at: %s", qPrintable(path));
        QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
    }


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


    // Load translators

    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale(), "qtbase", "_",
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        qApp->installTranslator(&qtTranslator);

    QTranslator translator;
    if (translator.load(QLocale(), qApp->applicationName(), "_", ":/i18n"))
        qApp->installTranslator(&translator);


    // Parse command line

    {
        auto opt_p = QCommandLineOption({"p", "plugin-dirs"},
                                        App::tr("Set the plugin dirs to use. Comma separated."),
                                        App::tr("directories"));
        auto opt_r = QCommandLineOption({"r", "report"},
                                        App::tr("Print report and quit."));
        auto opt_n = QCommandLineOption({"n", "no-load"},
                                        App::tr("Do not load enabled plugins."));

        QCommandLineParser parser;
        parser.addOptions({opt_p, opt_r, opt_n});
        parser.addPositionalArgument(App::tr("command"),
                                     App::tr("RPC command to send to the running instance."),
                                     App::tr("[command [params...]]"));
        parser.addVersionOption();
        parser.addHelpOption();
        parser.setApplicationDescription(App::tr("Launch Albert or control a running instance."));
        parser.process(qapp);

        if (!parser.positionalArguments().isEmpty())
            return RPCServer::trySendMessage(parser.positionalArguments().join(" ")) ? 0 : 1;

        if (parser.isSet(opt_r))
            printReportAndExit();


        // Create app

        for (const auto &line : report())
            DEBG << line;

        new App(parser.value(opt_p).split(',', Qt::SkipEmptyParts), !parser.isSet(opt_n));
    }


    // Run app

    app_instance->initialize();

    int return_value = qApp->exec();

    // Never quit with events in queue. 2024: Why? DeferredDelete is not handled anyway.
    QCoreApplication::processEvents();

    app_instance->finalize();

    delete app_instance;

    if (return_value == -1 && runDetachedProcess(qApp->arguments(), QDir::currentPath()))
        return_value = EXIT_SUCCESS;

    INFO << "Bye.";
    return return_value;
}

}
