// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/extensions/frontend.h"
#include "albert/util/standarditem.h"
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

    extension_registry.add(this);
    extension_registry.add(&plugin_registry);
    extension_registry.add(&plugin_provider);  // loads plugins
}

QString App::id() const { return "albert"; }

QString App::name() const { return "Albert"; }

QString App::description() const { return "Control the app."; }

void App::updateIndexItems()
{
    auto settings_item = StandardItem::make(
            "albert-settings", "Settings", "Open the Albert settings window", {":app_icon"},
            {{"albert-settings", "Open settings", [](){ showSettings(); }}}
    );

    auto quit_item = StandardItem::make(
            "albert-quit", "Quit Albert", "Quit this application", {":app_icon"},
            {{"albert-quit", "Quit Albert", [](){ quit(); }}}
    );

    auto restart_item = StandardItem::make(
            "albert-restart", "Restart Albert", "Restart this application", {":app_icon"},
            {{"albert-restart", "Restart Albert", [](){ restart(); }}}
    );

    vector<IndexItem> index_items;
    index_items.emplace_back(settings_item, "settings");
    index_items.emplace_back(settings_item, "preferences");
    index_items.emplace_back(quit_item, "quit");
    index_items.emplace_back(restart_item, "restart");
    setIndexItems(::move(index_items));
}
