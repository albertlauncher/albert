// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/frontend.h"
#include "albert/logging.h"
#include "albert/plugin/plugininstance.h"
#include "albert/plugin/pluginloader.h"
#include "albert/plugin/pluginmetadata.h"
#include "app.h"
#include "platform/platform.h"
#include "session.h"
#include "settings/pluginswidget/pluginswidget.h"
#include "settings/querywidget/querywidget.h"
#include "settings/settingswindow.h"
#include "trayicon.h"
#include <QHotkey>
#include <QMessageBox>
#include <QSettings>
using namespace albert;
using namespace std;

namespace {
static const char *STATE_LAST_USED_VERSION = "last_used_version";
static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";
static App * app_instance = nullptr;
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
    else
        app_instance = this;
}

App::~App() = default;

App *App::instance() { return app_instance; }

void App::initialize()
{
    platform::initPlatform();

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

bool App::trayEnabled() const
{
    return tray_icon.get();
}

void App::setTrayEnabled(bool enable)
{
    if (enable != trayEnabled())
    {
        tray_icon.reset(enable ? new TrayIcon : nullptr);
        settings()->setValue(CFG_SHOWTRAY, enable);
    }
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

        QObject::connect(frontend_, &Frontend::visibleChanged, this, [this](bool v){
            session.reset();  // make sure no multiple sessions are alive
            if(v)
                session = make_unique<Session>(query_engine_, *frontend_);
        });

        QObject::connect(&query_engine_, &QueryEngine::handlerRemoved, this, [this]{
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

const QHotkey *App::hotkey() const
{
    return hotkey_.get();
}

void App::setHotkey(unique_ptr<QHotkey> hotkey)
{
    if (hotkey->isRegistered())
    {
        hotkey_ = ::move(hotkey);
        connect(hotkey_.get(), &QHotkey::activated, frontend_, []{ toggle(); });
        settings()->setValue(CFG_HOTKEY, hotkey_->shortcut().toString());
    }
    else
        WARN << "Set unregistered hotkey.";
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
        connect(hotkey_.get(), &QHotkey::activated, frontend_, []{ toggle(); });
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
