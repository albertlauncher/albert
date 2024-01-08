// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "app.h"
#include "platform/platform.h"
#include <QHotkey>
#include <QMessageBox>
#include <QSettings>
using namespace albert;
using namespace std;


static const char *STATE_LAST_USED_VERSION = "last_used_version";
static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "qmlboxmodel";
static App * app_instance = nullptr;

App::App(const QStringList &additional_plugin_paths) :
    plugin_registry(extension_registry),
    query_engine(extension_registry),
    plugin_provider(additional_plugin_paths),
    settings_window(nullptr),
    app_query_handler(&extension_registry),
    plugin_query_handler(plugin_registry)
{
    if (app_instance)
        qFatal("No multiple app instances allowed");
    else
        app_instance = this;
}

App::~App()
{
    delete settings_window.get();

    // unload the frontend before plugins since it may have plugin objects in query
    for (auto &plugin : plugin_provider.frontendPlugins())
        plugin->loadUnregistered(&extension_registry, false);

    extension_registry.remove(&plugin_provider);  // unloads plugins
}

void App::initialize()
{
    platform::initPlatform();

    loadAnyFrontend();

    // Connect hotkey after! frontend has been loaded else segfaults
    QObject::connect(&hotkey, &Hotkey::activated, &hotkey, [](){ toggle(); });

    platform::initNativeWindow(frontend->winId());

    extension_registry.add(&plugin_provider);  // loads plugins
    extension_registry.add(&app_query_handler);
    extension_registry.add(&plugin_query_handler);

    notifyVersionChange();
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

QString App::loadFrontend(QtPluginLoader *loader)
{
    if (auto err = loader->loadUnregistered(&extension_registry); err.isNull()){
        if ((frontend = dynamic_cast<Frontend*>(loader->instance()))){
            frontend_plugin = loader;
            frontend->setEngine(&query_engine);
            return {};
        } else {
            loader->loadUnregistered(&extension_registry, false);
            return QString("Failed casting Plugin instance to albert::Frontend: %1").arg(loader->metaData().id);
        }
    } else
        return err;
}

void App::setFrontend(const QString &id)
{
    if (id != frontend_plugin->metaData().id){
        albert::settings()->setValue(CFG_FRONTEND_ID, id);
        QMessageBox msgBox(QMessageBox::Question, "Restart?",
                           "Changing the frontend needs a restart. Do you want to restart Albert?",
                           QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes)
            restart();
    }
}

void App::notifyVersionChange()
{
    auto state = albert::state();
    auto current_version = qApp->applicationVersion();

    // Move to state // TODO remove on next major version
    if (albert::settings()->contains(STATE_LAST_USED_VERSION)){
        state->setValue(STATE_LAST_USED_VERSION, albert::settings()->value(STATE_LAST_USED_VERSION));
        albert::settings()->remove(STATE_LAST_USED_VERSION);
    }

    auto last_used_version = state->value(STATE_LAST_USED_VERSION).toString();

    if (last_used_version.isNull()){  // First run
        QMessageBox(
            QMessageBox::Warning, "First run",
            "This is the first time you've launched Albert. Albert is plugin based. "
            "You have to enable some plugins you want to use.").exec();
        albert::showSettings();
    }
    else if (current_version.section('.', 1, 1) != last_used_version.section('.', 1, 1) )  // FIXME in first major version
        QMessageBox(QMessageBox::Information, "Major version changed",
                    QString("You are now using Albert %1. The major version changed. "
                            "Some parts of the API might have changed. Check the "
                            "<a href=\"https://albertlauncher.github.io/news/\">news</a>.")
                        .arg(current_version)).exec();

    if (last_used_version != current_version)
        state->setValue(STATE_LAST_USED_VERSION, current_version);
}
