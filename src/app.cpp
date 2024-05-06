// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/frontend.h"
#include "albert/logging.h"
#include "albert/plugin/plugininstance.h"
#include "albert/plugin/pluginloader.h"
#include "albert/plugin/pluginmetadata.h"
#include "app.h"
#include "platform/platform.h"
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
}

App::App(const QStringList &additional_plugin_paths, bool load_enabled) :
    plugin_registry(extension_registry, load_enabled),
    query_engine(extension_registry),
    plugin_provider(additional_plugin_paths),
    settings_window(nullptr),
    plugin_query_handler(plugin_registry),
    plugin_config_query_handler(plugin_registry)
{
    if (app_instance)
        qFatal("No multiple app instances allowed");
    else
        app_instance = this;
}

void App::initialize()
{
    platform::initPlatform();

    loadAnyFrontend();

    platform::initNativeWindow(frontend->winId());

    notifyVersionChange();

    // Connect hotkey after! frontend has been loaded else segfaults
    QObject::connect(&hotkey, &Hotkey::activated, &hotkey, [](){ toggle(); });

    extension_registry.registerExtension(&app_query_handler);
    extension_registry.registerExtension(&plugin_query_handler);
    extension_registry.registerExtension(&plugin_config_query_handler);
    extension_registry.registerExtension(&plugin_provider);  // loads plugins
}

void App::finalize()
{
    delete settings_window.get();

    disconnect(frontend, nullptr, this, nullptr);
    disconnect(&query_engine, nullptr, this, nullptr);
    session.reset();

    extension_registry.deregisterExtension(&plugin_provider);  // unloads plugins
    extension_registry.deregisterExtension(&plugin_config_query_handler);
    extension_registry.deregisterExtension(&plugin_query_handler);
    extension_registry.deregisterExtension(&app_query_handler);

    try {
        frontend_plugin->unload();
    } catch (const exception &e) {
        WARN << e.what();
    }
}

App *App::instance() { return app_instance; }

void App::loadAnyFrontend()
{
    auto frontend_plugins = plugin_provider.frontendPlugins();

    auto cfg_frontend = albert::settings()->value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();
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
        PluginRegistry::staticDI.dependencies = {};
        loader->load();

        plugin_registry.staticDI.loader = loader;
        auto * inst = loader->createInstance();
        if (!inst)
            return "Plugin loader returned null instance";

        frontend = dynamic_cast<Frontend*>(loader->createInstance());
        if (!frontend)
            return QString("Failed casting Plugin instance to albert::Frontend: %1").arg(loader->metaData().id);

        frontend_plugin = loader;

        QObject::connect(frontend, &Frontend::visibleChanged, this, [this](bool v){
            session.reset();  // make sure no multiple sessions are alive
            if(v)
                session = make_unique<Session>(query_engine, *frontend);
        });

        QObject::connect(&query_engine, &QueryEngine::handlerRemoved, this, [this]{
            if (frontend->isVisible())
            {
                session.reset();
                session = make_unique<Session>(query_engine, *frontend);
            }
        });
        return {};
    } catch (const exception &e) {
        return QString::fromStdString(e.what());
    } catch (...) {
        return "Unknown exception";
    }
}

void App::setFrontend(const QString &id)
{
    if (id != frontend_plugin->metaData().id)
    {
        albert::settings()->setValue(CFG_FRONTEND_ID, id);

        auto text = QCoreApplication::translate(
            "App",
            "Changing the frontend requires a restart. "
            "Do you want to restart Albert?"
            );

        if (QMessageBox::question(nullptr, qApp->applicationDisplayName(), text) == QMessageBox::Yes)
            restart();
    }
}

void App::notifyVersionChange()
{
    auto state = albert::state();
    auto current_version = qApp->applicationVersion();
    auto last_used_version = state->value(STATE_LAST_USED_VERSION).toString();

    // First run
    if (last_used_version.isNull())
    {
        auto text = QCoreApplication::translate(
            "App",
            "This is the first time you've launched Albert. Albert is plugin based. "
            "You have to enable some plugins you want to use."
            );

        QMessageBox::information(nullptr, qApp->applicationDisplayName(), text);

        albert::showSettings();
    }
    else if (current_version.section('.', 1, 1) != last_used_version.section('.', 1, 1) )  // FIXME in first major version
    {
        auto text = QCoreApplication::translate(
            "App",
            "You are now using Albert %1. The major version changed. "
            "Some parts of the API might have changed. "
            "Check the <a href=\"https://albertlauncher.github.io/news/\">news</a>."
            ).arg(current_version);

        QMessageBox::information(nullptr, qApp->applicationDisplayName(), text);
    }

    if (last_used_version != current_version)
        state->setValue(STATE_LAST_USED_VERSION, current_version);
}
