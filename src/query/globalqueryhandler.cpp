// Copyright (c) 2023-2025 Manuel Schneider

#include "globalqueryhandler.h"
#include "query.h"
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() = default;

void GlobalQueryHandler::handleTriggerQuery(Query &query)
{
    auto rank_items = handleGlobalQuery(query);
    applyUsageScore(rank_items);
    ranges::sort(rank_items, greater());

    vector<shared_ptr<Item>> items;
    items.reserve(rank_items.size());
    for (auto &match : rank_items)
        items.push_back(::move(match.item));

    query.add(::move(items));
}

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery() { return {}; }
