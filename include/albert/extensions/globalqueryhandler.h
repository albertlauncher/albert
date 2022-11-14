// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "queryhandler.h"

namespace albert
{

using Score = uint16_t;
constexpr Score MAX_SCORE = std::numeric_limits<Score>::max();

class ALBERT_EXPORT RankItem
{
public:
    RankItem(std::shared_ptr<Item> item, Score score);
    std::shared_ptr<Item> item;
    Score score;
};


/// Global search handler. Do not use this for long running queries
class ALBERT_EXPORT GlobalQueryHandler : public QueryHandler
{
public:
    virtual std::vector<RankItem> rankItems(const Query &query) const = 0;  /// Called on global search
    void handleQuery(Query &query) const final;  /// Sorts items returned by rankedItems
};

}


