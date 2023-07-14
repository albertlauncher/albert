// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "app.h"
#include <QHotkey>
#include <QMessageBox>
#include <QSettings>
#ifdef Q_OS_MAC
#include <objc/objc-runtime.h>
#endif
using namespace albert;
using namespace std;


static const char *CFG_FRONTEND_ID = "frontend";
static const char *DEF_FRONTEND_ID = "widgetsboxmodel";
static App * app_instance = nullptr;

App::App(const QStringList &additional_plugin_paths) :
    plugin_registry(extension_registry),
    query_engine(extension_registry),
    plugin_provider(additional_plugin_paths),
    settings_window(nullptr),
    plugin_query_handler(plugin_registry)
{
    if (app_instance)
        qFatal("No multiple app instances allowed");
    else
        app_instance = this;
}

App::~App()
{
    // unload the frontend before plugins since it may have plugin objects
    for (auto &plugin : plugin_provider.frontendPlugins())
        plugin->unload(&extension_registry);

    delete settings_window.get();
    extension_registry.remove(&plugin_provider);  // unloads plugins
}

void App::initialize()
{
    loadAnyFrontend();
    applyPlatformWindowQuirks(frontend);

    extension_registry.add(&plugin_provider);  // loads plugins
    extension_registry.add(&app_query_handler);
    extension_registry.add(&plugin_query_handler);
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

void App::applyPlatformWindowQuirks(Frontend *f)
{
#ifdef Q_OS_MAC
    WId window_id = f->winId();
    auto call_objc = [](objc_object* object, const char* selector){
        return ((objc_object* (*)(::id, SEL))objc_msgSend)(object, sel_registerName(selector));
    };
    auto call_objc_1 = [](objc_object* object, const char *selector, const auto &param){
        return ((objc_object* (*)(::id, SEL, decltype(param)))objc_msgSend)(object, sel_registerName(selector), param);
    };
    auto *ns_view_object = reinterpret_cast<objc_object *>(window_id);
    objc_object *ns_window = call_objc(ns_view_object, "window");
    //    call_objc_1(ns_window, "setCollectionBehavior:",
    ((objc_object* (*)(::id, SEL, int))objc_msgSend)(ns_window, sel_registerName("setCollectionBehavior:"),
        0  // NSWindowCollectionBehaviorDefault - The window appears in only one space at a time.
         //                | 1 << 0   // NSWindowCollectionBehaviorCanJoinAllSpaces - The window appears in all spaces.
         | 1 << 1   // NSWindowCollectionBehaviorMoveToActiveSpace - When the window becomes active, move it to the active space instead of switching spaces.
        //        | 1 << 2   // NSWindowCollectionBehaviorManaged - The window participates in Spaces and Exposé.
        //        | 1 << 3   // NSWindowCollectionBehaviorTransient - The window floats in Spaces and hides in Exposé.
        //        | 1 << 4   // NSWindowCollectionBehaviorStationary - Exposé doesn’t affect the window, so it stays visible and stationary, like the desktop window.
        //        | 1 << 5   // NSWindowCollectionBehaviorParticipatesInCycle - The window participates in the window cycle for use with the Cycle Through Windows menu item.
        //        | 1 << 6   // NSWindowCollectionBehaviorIgnoresCycle - The window isn’t part of the window cycle for use with the Cycle Through Windows menu item.
        //        | 1 << 7   // NSWindowCollectionBehaviorFullScreenPrimary - The window can enter full-screen mode.
        //        | 1 << 8   // NSWindowCollectionBehaviorFullScreenAuxiliary - The window can display on the same space as the full-screen window.
        //        | 1 << 9   // NSWindowCollectionBehaviorFullScreenNone - The window doesn’t support full-screen mode.
        //        | 1 << 11  // NSWindowCollectionBehaviorFullScreenAllowsTiling - The window can be a secondary full screen tile even if it can’t be a full screen window itself.
        //        | 1 << 12  // NSWindowCollectionBehaviorFullScreenDisallowsTiling - The window doesn’t support being a full-screen tile window, but may support being a full-screen window.
    );
    call_objc_1(ns_window, "setAnimationBehavior:", 2);  // NSWindowAnimationBehaviorNone
//    ((objc_object* (*)(id, SEL, int))objc_msgSend)(ns_window, sel_registerName("setAnimationBehavior:"), 2);
#endif
}


QString App::loadFrontend(QtPluginLoader *loader)
{
    loader->load_(&extension_registry);
    if (loader->state() == PluginState::Loaded){
        if ((frontend = dynamic_cast<Frontend*>(loader->instance()))){
            frontend->setEngine(&query_engine);
            return {};
        } else {
            loader->unload(&extension_registry);
            return QString("Failed casting Plugin instance to albert::Frontend: %1").arg(loader->metaData().id);
        }
    } else
        return loader->stateInfo();
}


void App::setFrontend(const QString &id)
{
    if (id != frontend->id()){
        albert::settings()->setValue(CFG_FRONTEND_ID, id);
        QMessageBox msgBox(QMessageBox::Question, "Restart?",
                           "Changing the frontend needs a restart. Do you want to restart Albert?",
                           QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes)
            restart();
    }
}
