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

    static void setWeight(double weight);
    static void setScores(std::map<std::pair<QString,QString>,double> scores);
    void applyUsageScores(std::vector<albert::RankItem> &rank_items) const;
    std::vector<albert::RankItem> handleQuery(const albert::GlobalQueryHandler::Query&) const;

private:
    static std::shared_mutex m;
    static std::map<std::pair<QString,QString>,double> usage_scores;
    static double usage_weight;
};
