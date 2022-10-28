// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "queryhandler.h"

namespace albert
{
/// Global search handler. Do not use this for long running queries
struct ALBERT_EXPORT GlobalQueryHandler : public QueryHandler
{
    virtual std::vector<Match> rankedItems(const albert::Query &query) const = 0;  /// Called on global search
    void handleQuery(albert::Query &query) const override final;  /// Sorts items returned by rankedItems
};
}


