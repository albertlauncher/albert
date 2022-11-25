// Copyright (c) 2022 Manuel Schneider

#include "albert/util/standarditem.h"
#include "albert/extensions/frontend.h"
#include "app.h"
using namespace albert;
using namespace std;

App::App(const QStringList &additional_plugin_paths) :
        extension_registry(),
        query_engine(extension_registry),
        plugin_provider(extension_registry, additional_plugin_paths),
        terminal_provider(),
        settings_window(nullptr){}

App::~App()
{
    delete settings_window.get();
}

void App::initialize()
{
    extension_registry.add(&plugin_provider);
    extension_registry.add(this);
    plugin_provider.loadFrontend();
    plugin_provider.frontend()->setEngine(&query_engine);
    plugin_provider.loadEnabledPlugins();
}

QString App::id() const
{
    return "albert";
}

QString App::name() const
{
    return "Albert";
}

QString App::description() const
{
    return "Control the app.";
}

vector<IndexItem> App::indexItems() const
{
    auto settings_item = make_shared<StandardItem>(
            "albert-settings", "Settings", "Open the Albert settings window", QStringList{":app_icon"},
            Actions{{"albert-settings", "Open settings", [](){ showSettings(); }}}
    );

    auto quit_item = make_shared<StandardItem>(
            "albert-quit", "Quit Albert", "Quit this application", QStringList{":app_icon"},
            Actions{{"albert-quit", "Quit Albert", [](){ quit(); }}}
    );

    auto restart_item = make_shared<StandardItem>(
            "albert-restart", "Restart Albert", "Restart this application", QStringList{":app_icon"},
            Actions{{"albert-restart", "Restart Albert", [](){ restart(); }}}
    );

    vector<IndexItem> items;
    items.emplace_back(settings_item, "settings");
    items.emplace_back(settings_item, "preferences");
    items.emplace_back(quit_item, "quit");
    items.emplace_back(restart_item, "restart");
    return items;
}
