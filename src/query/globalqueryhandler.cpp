// Copyright (c) 2023-2024 Manuel Schneider

#include "globalqueryhandler.h"
#include "query.h"
#include "usagedatabase.h"
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() = default;

void GlobalQueryHandler::applyUsageScore(vector<RankItem> *rankItems) const
{ UsageHistory::applyScores(id(), *rankItems); }

void GlobalQueryHandler::handleTriggerQuery(Query *query)
{
    auto rank_items = handleGlobalQuery(query);
    applyUsageScore(&rank_items);
    ranges::sort(rank_items, std::greater());

    vector<shared_ptr<Item>> items;
    items.reserve(rank_items.size());
    for (auto &match : rank_items)
        items.push_back(::move(match.item));

    query->add(::move(items));
}

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery(const Query *)
{ return {}; }
