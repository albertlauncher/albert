// Copyright (c) 2023-2025 Manuel Schneider

#include "app.h"
#include "globalqueryhandler.h"
#include "query.h"
#include "queryengine.h"
#include "usagescoring.h"
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() = default;

void GlobalQueryHandler::applyUsageScore(vector<RankItem> &rank_items) const
{ App::instance()->queryEngine().usageScoring().modifyMatchScores(*this, rank_items); }

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

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery()
{ return {}; }
