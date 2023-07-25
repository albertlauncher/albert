// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "globalqueryhandlerprivate.h"
#include "usagedatabase.h"
using namespace albert;
using namespace std;

GlobalQueryHandler::GlobalQueryHandler() : d(new GlobalQueryHandlerPrivate(this)) {}

GlobalQueryHandler::~GlobalQueryHandler() = default;

void GlobalQueryHandler::applyUsageScore(vector<RankItem> *rankItems) const
{ UsageHistory::applyScores(id(), *rankItems); }

void GlobalQueryHandler::handleTriggerQuery(TriggerQuery *query) const
{
    struct : public GlobalQuery {
        TriggerQuery *query;
        QString trigger() const { return query->trigger(); }
        QString string() const { return query->string(); }
        const bool &isValid() const { return query->isValid(); }
    } gq;
    gq.query = query;

    vector<RankItem> rank_items = handleGlobalQuery(&gq);
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
