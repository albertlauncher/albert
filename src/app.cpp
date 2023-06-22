// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/extensions/frontend.h"
#include "app.h"
#include <QHotkey>
using namespace albert;
using namespace std;

App::App(const QStringList &additional_plugin_paths) :
    extension_registry(),
    plugin_registry(extension_registry),
    query_engine(extension_registry),
    plugin_provider(extension_registry, additional_plugin_paths),
    settings_window(nullptr){}

App::~App()
{
    delete settings_window.get();
}

void App::initialize()
{
    plugin_provider.loadFrontend();
    plugin_provider.frontend()->setEngine(&query_engine);

    extension_registry.add(&plugin_registry);
    extension_registry.add(&plugin_provider);  // loads plugins
    extension_registry.add(&app_query_handler);
}
