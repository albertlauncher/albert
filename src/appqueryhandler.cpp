// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/util/standarditem.h"
#include "appqueryhandler.h"
using namespace albert;
using namespace std;

QString AppQueryHandler::id() const { return "albert"; }

QString AppQueryHandler::name() const { return "Albert"; }

QString AppQueryHandler::description() const { return "Control the app"; }

void AppQueryHandler::updateIndexItems()
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
