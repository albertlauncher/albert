// Copyright (c) 2023-2025 Manuel Schneider

#include "albert.h"
#include "globalqueryhandler.h"
#include "query.h"
#include "usagescoring.h"
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() = default;

void GlobalQueryHandler::handleTriggerQuery(Query &query)
{
    auto rank_items = handleGlobalQuery(query);

    usageScoring().modifyMatchScores(id(), rank_items);

    auto begin = ::begin(rank_items);
    auto end = ::end(rank_items);
    auto mid = begin + 20;

    vector<shared_ptr<Item>> items;
    items.reserve(mid - begin);

    // Partially sort the visible items for fast response times
    if (mid < end)
    {
        ranges::partial_sort(begin, mid, end, greater());
        for (;begin != mid; ++begin)
            items.emplace_back(::move(begin->item));
        query.add(::move(items));
    }

    items.clear();
    items.reserve(end - begin);

    ranges::sort(begin, end, greater());
    for (;begin != end; ++begin)
        items.emplace_back(::move(begin->item));
    query.add(::move(items));
}

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery() { return {}; }
