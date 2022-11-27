// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/globalqueryhandler.h"
#include "usagedatabase.h"
#include <algorithm>
#include <shared_mutex>
#include <utility>
using namespace std;
using namespace albert;

static shared_mutex m;
static map<pair<QString,QString>,double> usage_scores;
static double usage_weight;

RankItem::RankItem(shared_ptr<Item> item, Score score):
    item(std::move(item)), score(score)
{

}

void albert::GlobalQueryHandler::handleQuery(Query &query) const
{
    std::vector<RankItem> &&rank_items = rankItems(query.string(), query.isValid());
    sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){ return a.score > b.score; });

    // TODO c++20 ranges::view
    std::vector<shared_ptr<Item>> items;
    items.reserve(rank_items.size());
    for (auto &match : rank_items)
        items.push_back(std::move(match.item));

    query.add(std::move(items));
}

void GlobalQueryHandler::applyUsageScores(vector<RankItem> &rank_items) const
{
    shared_lock lock(m);
    // https://github.com/albertlauncher/albert/issues/695
    for (auto & rank_item : rank_items){
        try {
            rank_item.score = (Score)((usage_scores.at(make_pair(this->id(),rank_item.item->id())) * usage_weight
                              + (double)rank_item.score/(double)MAX_SCORE * (1-usage_weight)) * MAX_SCORE);
        } catch (const out_of_range &){}
    }
}

void GlobalQueryHandler::setScores(std::map<std::pair<QString,QString>,double> scores)
{
    unique_lock lock(m);
    usage_scores = std::move(scores);

}

void GlobalQueryHandler::setWeight(double weight)
{
    unique_lock lock(m);
    usage_weight = weight;

}