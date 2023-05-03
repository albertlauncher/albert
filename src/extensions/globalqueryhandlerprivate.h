// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extensions/queryhandler.h"
#include <map>
#include <shared_mutex>
#include <utility>
#include <vector>


class GlobalQueryHandlerPrivate final
{
public:
    GlobalQueryHandlerPrivate(albert::GlobalQueryHandler *q);
    ~GlobalQueryHandlerPrivate();
    albert::GlobalQueryHandler * q;

    static void setPrioritizePerfectMatch(bool val);
    static void setScores(std::map<std::pair<QString,QString>,albert::RankItem::Score> scores);

    void applyUsageScores(std::vector<albert::RankItem> &rank_items) const;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::GlobalQueryHandler::GlobalQuery&) const;

private:
    static std::shared_mutex m;
    static std::map<std::pair<QString,QString>,albert::RankItem::Score> usage_scores;
    static bool prio_perfect_match;
};
