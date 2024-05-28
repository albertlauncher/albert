// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/util.h"
#include "albert/frontend.h"
#include "albert/logging.h"
#include "albert/plugin/plugininstance.h"
#include "albert/plugin/pluginloader.h"
#include "messagehandler.h"
#include "albert/plugin/pluginmetadata.h"
#include "app.h"
#include "albert/util/iconprovider.h"
#include "platform/platform.h"
#include "report.h"
#include "session.h"
#include "settings/pluginswidget/pluginswidget.h"
#include "settings/querywidget/querywidget.h"
#include "settings/settingswindow.h"
#include <QHotkey>
#include <QCommandLineParser>
#include <QMenu>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTranslator>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
Q_LOGGING_CATEGORY(AlbertLoggingCategory, "albert")
using namespace albert;
using namespace std;


namespace {
static App * app_instance = nullptr;
// albert::App *app;
static const char *STATE_LAST_USED_VERSION = "last_used_version";
static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";
static const char* CFG_SHOWTRAY = "showTray";
static const bool  DEF_SHOWTRAY = true;
static const char *CFG_HOTKEY = "hotkey";
static const char *DEF_HOTKEY = "Ctrl+Space";
}

App::App(const QStringList &additional_plugin_paths, bool load_enabled) :
    plugin_registry_(extension_registry_, load_enabled),
    query_engine_(extension_registry_),
    plugin_provider(additional_plugin_paths),
    settings_window(nullptr),
    plugin_query_handler(plugin_registry_),
    plugin_config_query_handler(plugin_registry_)
{
    if (app_instance)
        qFatal("No multiple app instances allowed");

    app_instance = this;
    // app = this;
}

App::~App() = default;

App *App::instance()
{
    return app_instance;
}

void App::initialize()
{
    platform::initPlatform();

    if (qtTranslator.load(QLocale(), "qtbase", "_",
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        qApp->installTranslator(&qtTranslator);

    if (translator.load(QLocale(), qApp->applicationName(), "_", ":/i18n"))
        qApp->installTranslator(&translator);

    loadAnyFrontend();

    platform::initNativeWindow(frontend_->winId());

    setTrayEnabled(settings()->value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool());

    notifyVersionChange();

    // Connect hotkey after! frontend has been loaded else segfaults
    initHotkey();

    extension_registry_.registerExtension(&app_query_handler);
    extension_registry_.registerExtension(&plugin_query_handler);
    extension_registry_.registerExtension(&plugin_config_query_handler);
    extension_registry_.registerExtension(&plugin_provider);  // loads plugins
}

void App::finalize()
{
    delete settings_window.get();

    disconnect(frontend_, nullptr, this, nullptr);
    disconnect(&query_engine_, nullptr, this, nullptr);
    session.reset();

    extension_registry_.deregisterExtension(&plugin_provider);  // unloads plugins
    extension_registry_.deregisterExtension(&plugin_config_query_handler);
    extension_registry_.deregisterExtension(&plugin_query_handler);
    extension_registry_.deregisterExtension(&app_query_handler);

    try {
        frontend_plugin->unload();
    } catch (const exception &e) {
        WARN << e.what();
    }
}

void App::initHotkey()
{
    if (!QHotkey::isPlatformSupported())
    {
        INFO << "Hotkeys are not supported on this platform.";
        return;
    }

    auto s_hotkey = settings()->value(CFG_HOTKEY, DEF_HOTKEY).toString();
    auto kc_hotkey = QKeySequence::fromString(s_hotkey)[0];

    if (auto hotkey = make_unique<QHotkey>(kc_hotkey);
        hotkey->setRegistered(true))
    {
        hotkey_ = ::move(hotkey);
        connect(hotkey_.get(), &QHotkey::activated,
                frontend_, []{ App::instance()->toggle(); });
        INFO << "Hotkey set to" << s_hotkey;
    }
    else
    {
        auto text = QT_TR_NOOP("Failed to set the hotkey '%1'");
        WARN << QString(text).arg(s_hotkey);
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                             tr(text).arg(QKeySequence(kc_hotkey)
                                          .toString(QKeySequence::NativeText)));
        showSettings();
    }
}

void App::initAppDirectories()
{
    for (const auto &path : { cacheLocation(), configLocation(), dataLocation() })
    {
        if (!QDir(path).mkpath("."))
            qFatal("Failed creating config dir at: %s", qPrintable(path));
        QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
    }
}

void App::initPRC()
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
}

bool App::trayEnabled() const
{
    return tray_icon.get();
}

void App::setTrayEnabled(bool enable)
{
    if (enable && !trayEnabled())
    {
        settings()->setValue(CFG_SHOWTRAY, enable);

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
    else if (!enable && trayEnabled())
    {
        settings()->setValue(CFG_SHOWTRAY, enable);
        tray_icon.reset();
        tray_menu.reset();
    }
}

const QHotkey *App::hotkey() const
{
    return hotkey_.get();
}

void App::setHotkey(unique_ptr<QHotkey> hotkey)
{
    if (hotkey->isRegistered())
    {
        hotkey_ = ::move(hotkey);
        connect(hotkey_.get(), &QHotkey::activated,
                frontend_, []{ App::instance()->toggle(); });
        settings()->setValue(CFG_HOTKEY, hotkey_->shortcut().toString());
    }
    else
        WARN << "Set unregistered hotkey. Ignoring.";
}

QStringList App::availableFrontends()
{
    QStringList ret;
    for (const auto *loader : plugin_provider.frontendPlugins())
        ret << loader->metaData().name;
    return ret;
}

QString App::currentFrontend()
{
    return frontend_plugin->metaData().name;
}

void App::setFrontend(uint i)
{
    auto fp = plugin_provider.frontendPlugins().at(i);
    settings()->setValue(CFG_FRONTEND_ID, fp->metaData().id);

    auto text = tr("Changing the frontend requires a restart. "
                   "Do you want to restart Albert?");

    if (QMessageBox::question(nullptr, qApp->applicationDisplayName(), text) == QMessageBox::Yes)
        restart();
}

Frontend *App::frontend()
{
    return frontend_;
}

void App::loadAnyFrontend()
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

QString App::loadFrontend(PluginLoader *loader)
{
    try {
        PluginRegistry::staticDI.loader = loader;
        loader->load();

        plugin_registry_.staticDI.loader = loader;
        auto * inst = loader->createInstance();
        if (!inst)
            return "Plugin loader returned null instance";

        frontend_ = dynamic_cast<Frontend*>(loader->createInstance());
        if (!frontend_)
            return QString("Failed casting Plugin instance to albert::Frontend: %1").arg(loader->metaData().id);

        frontend_plugin = loader;

        connect(frontend_, &Frontend::visibleChanged, this, [this](bool v){
            session.reset();  // make sure no multiple sessions are alive
            if(v)
                session = make_unique<Session>(query_engine_, *frontend_);
        });

        connect(&query_engine_, &QueryEngine::handlerRemoved, this, [this]{
            if (frontend_->isVisible())
            {
                session.reset();
                session = make_unique<Session>(query_engine_, *frontend_);
            }
        });
        return {};
    } catch (const exception &e) {
        return QString::fromStdString(e.what());
    } catch (...) {
        return "Unknown exception";
    }
}

void App::notifyVersionChange()
{
    auto s = state();
    auto current_version = qApp->applicationVersion();
    auto last_used_version = s->value(STATE_LAST_USED_VERSION).toString();

    // First run
    if (last_used_version.isNull())
    {
        auto text = tr("This is the first time you've launched Albert. Albert is plugin based. "
                       "You have to enable some plugins you want to use.");

        QMessageBox::information(nullptr, qApp->applicationDisplayName(), text);

        showSettings();
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

void App::showSettings(QString plugin_id)
{
    if (!settings_window)
        settings_window = new SettingsWindow(*this);
    hide();
    settings_window->bringToFront(plugin_id);
}

void App::show(const QString &text)
{
    if (!text.isNull())
        frontend()->setInput(text);
    frontend()->setVisible(true);
}

void App::hide()
{
    frontend()->setVisible(false);
}

void App::toggle()
{
    frontend()->setVisible(!frontend()->isVisible());
}

void App::restart()
{
    QMetaObject::invokeMethod(qApp, "exit", Qt::QueuedConnection, Q_ARG(int, -1));
}

void App::quit()
{
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

TerminalProvider &App::terminal()
{
    return terminal_provider_;
}

PluginsWidget *App::makePluginsWidget()
{
    return new PluginsWidget(plugin_registry_);
}

QWidget *App::makeQueryWidget()
{
    return new QueryWidget(query_engine_);
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


    // Set locale from env vars (why?)

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

        new App(parser.value(opt_p).split(',', Qt::SkipEmptyParts), !parser.isSet(opt_n));
    }


    // Run app

    for (const auto &line : report())
        DEBG << line;

    App::instance()->initialize();
    int return_value = qApp->exec();
    App::instance()->finalize();
    App::instance()->deleteLater();
    QCoreApplication::processEvents(); // Never quit with events in queue

    if (return_value == -1 && runDetachedProcess(qApp->arguments(), QDir::currentPath()))
        return_value = EXIT_SUCCESS;

    INFO << "Bye.";
    return return_value;
}

}
