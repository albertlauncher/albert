// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/query/globalqueryhandler.h"
#include "usagedatabase.h"
using namespace albert;
using namespace std;


void GlobalQueryHandler::applyUsageScore(vector<RankItem> *rankItems) const
{ UsageHistory::applyScores(id(), *rankItems); }

void GlobalQueryHandler::handleTriggerQuery(Query *query)
{
    auto rank_items = handleGlobalQuery(query);
    applyUsageScore(&rank_items);
    sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){
        if (a.score == b.score)
            return a.item->text() > b.item->text();
        else
            return a.score > b.score;
    });

    vector<shared_ptr<Item>> items; // TODO c++20 ranges::view
    items.reserve(rank_items.size());
    for (auto &match : rank_items)
        items.push_back(::move(match.item));

    query->add(::move(items));
}

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery(const Query *) const
{ return {}; }
