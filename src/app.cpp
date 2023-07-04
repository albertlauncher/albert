// Copyright (c) 2023 Manuel Schneider

#include "app.h"
#include "albert/extension/frontend/frontend.h"
#include <QHotkey>
using namespace albert;
using namespace std;

static App * app_instance = nullptr;

App::App(const QStringList &additional_plugin_paths) :
    extension_registry(),
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
    delete settings_window.get();
}

void App::initialize()
{
    plugin_provider.loadFrontend();
    plugin_provider.frontend()->setEngine(&query_engine);

    extension_registry.add(&plugin_provider);  // loads plugins

    extension_registry.add(&app_query_handler);
    extension_registry.add(&plugin_query_handler);
}


App *App::instance() { return app_instance; }
