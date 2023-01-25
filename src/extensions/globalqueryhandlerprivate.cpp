// Copyright (c) 2023 Manuel Schneider

#include "globalqueryhandlerprivate.h"
#include <mutex>
using namespace std;
using namespace albert;


std::shared_mutex GlobalQueryHandlerPrivate::m;
std::map<std::pair<QString,QString>,double> GlobalQueryHandlerPrivate::usage_scores;
double GlobalQueryHandlerPrivate::usage_weight;

GlobalQueryHandlerPrivate::GlobalQueryHandlerPrivate(GlobalQueryHandler *q_) : q(q_) {}

GlobalQueryHandlerPrivate::~GlobalQueryHandlerPrivate() = default;

void GlobalQueryHandlerPrivate::setWeight(double weight)
{
    unique_lock lock(m);
    usage_weight = weight;
}

void GlobalQueryHandlerPrivate::setScores(std::map<std::pair<QString, QString>, double> scores)
{
    unique_lock lock(m);
    usage_scores = std::move(scores);
}

void GlobalQueryHandlerPrivate::applyUsageScores(vector<RankItem> &rank_items) const
{
    shared_lock lock(m);
    // https://github.com/albertlauncher/albert/issues/695
    auto MAX_SCOREf = (double)RankItem::MAX_SCORE;
    for (auto & rank_item : rank_items){
        try {
            rank_item.score = (usage_weight * usage_scores.at(make_pair(q->id(), rank_item.item->id()))
                                           * MAX_SCOREf + (1.0-usage_weight) * (double)rank_item.score);
        } catch (const out_of_range &){
            rank_item.score = ((1.0-usage_weight) * (double)rank_item.score);
        }
    }
}

vector<RankItem> GlobalQueryHandlerPrivate::handleQuery(const GlobalQueryHandler::Query &query) const
{
    vector<RankItem> rank_items = q->handleQuery(query);
    applyUsageScores(rank_items);
    return rank_items;
}
