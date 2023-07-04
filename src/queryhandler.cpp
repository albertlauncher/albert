// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extension/queryhandler/queryhandler.h"
#include "globalqueryhandlerprivate.h"
using namespace std;
using namespace albert;

void QueryHandler::handleTriggerQuery(TriggerQuery *query) const
{
    vector<RankItem> &&rank_items = d->handleGlobalQuery(dynamic_cast<const GlobalQuery*>(query));
    sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){ return a.score > b.score; });

    // TODO c++20 ranges::view
    vector<shared_ptr<Item>> items;
    items.reserve(rank_items.size());
    for (auto &match : rank_items)
        items.push_back(::move(match.item));

    query->add(::move(items));
}
