// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "appqueryhandler.h"
using namespace albert;
using namespace std;

QString AppQueryHandler::id() const { return "albert"; }

QString AppQueryHandler::name() const { return "Albert"; }

QString AppQueryHandler::description() const { return "Control the app"; }

void AppQueryHandler::updateIndexItems()
{
    auto settings_item = StandardItem::make(
        "albert-settings",
        "Settings",
        "Open the Albert settings window",
        {":app_icon"},
        {{"albert-settings", "Open settings", [](){ showSettings(); }}}
    );

    auto quit_item = StandardItem::make(
        "albert-quit",
        "Quit Albert",
        "Quit this application",
        {":app_icon"},
        {{"albert-quit", "Quit Albert", [](){ quit(); }}}
    );

    auto restart_item = StandardItem::make(
        "albert-restart",
        "Restart Albert",
        "Restart this application",
        {":app_icon"},
        {{"albert-restart", "Restart Albert", [](){ restart(); }}}
    );

    auto config = StandardItem::make(
        "albert-config",
        "Albert config location",
        albert::configLocation(),
        {":app_icon"},
        {{"open", "Open", [](){ albert::openUrl(albert::configLocation()); }}}
        );

    auto data = StandardItem::make(
        "albert-data",
        "Albert data location",
        albert::dataLocation(),
        {":app_icon"},
        {{"open", "Open", [](){ albert::openUrl(albert::dataLocation()); }}}
        );

    auto cache = StandardItem::make(
        "albert-cache",
        "Albert cache location",
        albert::cacheLocation(),
        {":app_icon"},
        {{"open", "Open", [](){ albert::openUrl(albert::cacheLocation()); }}}
        );

    vector<IndexItem> index_items;
    index_items.emplace_back(settings_item, "settings");
    index_items.emplace_back(settings_item, "preferences");
    index_items.emplace_back(quit_item, "quit");
    index_items.emplace_back(restart_item, "restart");
    index_items.emplace_back(config, "config");
    index_items.emplace_back(data, "data");
    index_items.emplace_back(cache, "cachenf");
    setIndexItems(::move(index_items));
}
