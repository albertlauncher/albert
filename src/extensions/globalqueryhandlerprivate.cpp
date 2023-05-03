// Copyright (c) 2023 Manuel Schneider

#include "globalqueryhandlerprivate.h"
#include <mutex>
using namespace std;
using namespace albert;


std::shared_mutex GlobalQueryHandlerPrivate::m;
std::map<std::pair<QString,QString>,RankItem::Score> GlobalQueryHandlerPrivate::usage_scores;
bool GlobalQueryHandlerPrivate::prio_perfect_match;

GlobalQueryHandlerPrivate::GlobalQueryHandlerPrivate(GlobalQueryHandler *q_) : q(q_) {}

GlobalQueryHandlerPrivate::~GlobalQueryHandlerPrivate() = default;

void GlobalQueryHandlerPrivate::setPrioritizePerfectMatch(bool val)
{
    unique_lock lock(m);
    prio_perfect_match = val;
}

void GlobalQueryHandlerPrivate::setScores(std::map<std::pair<QString, QString>, RankItem::Score> scores)
{
    unique_lock lock(m);
    usage_scores = std::move(scores);
}

void GlobalQueryHandlerPrivate::applyUsageScores(vector<RankItem> &rank_items) const
{
    shared_lock lock(m);
    // https://github.com/albertlauncher/albert/issues/695
    for (auto & rank_item : rank_items){

        if (prio_perfect_match && rank_item.score == RankItem::MAX_SCORE){  // Prefer exact matches
            rank_item.score = RankItem::MAX_SCORE/3*2;
            try {
                rank_item.score += usage_scores.at(make_pair(q->id(), rank_item.item->id()))/3;
            } catch (const out_of_range &){}
        } else {
            try {
                rank_item.score = usage_scores.at(make_pair(q->id(), rank_item.item->id()))/3 + RankItem::MAX_SCORE/3;
            } catch (const out_of_range &){
                rank_item.score = rank_item.score / 3;
            }
        }
    }
}

vector<RankItem> GlobalQueryHandlerPrivate::handleGlobalQuery(const GlobalQueryHandler::GlobalQuery &query) const
{
    vector<RankItem> rank_items = q->handleGlobalQuery(query);
    applyUsageScores(rank_items);
    return rank_items;
}
