// Copyright (c) 2023-2025 Manuel Schneider

#include "threadedqueryhandler.h"
#include "usagescoring.h"
#include <QCoroGenerator>
#include <ranges>
using namespace albert;
using namespace std;

ThreadedQueryHandler::~ThreadedQueryHandler() {}

ItemGenerator ThreadedQueryHandler::items(Query &query)
{
    auto rank_items = rankItems(query);
    query.usageScoring().modifyMatchScores(id(), rank_items);
    return lazySort(::move(rank_items));
}

ItemGenerator ThreadedQueryHandler::lazySort(vector<RankItem> rank_items)
{
    while(!rank_items.empty())
    {
        // Partial sort the items incrementally in reverse order (for cheap "pop_n")
        auto reverse_view = rank_items | views::reverse;
        auto take_view = reverse_view | views::take(10);
        ranges::partial_sort(reverse_view, take_view.end(), greater{});

        // Yield chunk
        auto item_view = take_view | views::transform(&RankItem::item);
        vector<shared_ptr<Item>> item_vector {
            make_move_iterator(begin(item_view)),
            make_move_iterator(end(item_view))
        };

        // Cheap pop_n
        rank_items.erase(rank_items.end() - take_view.size(),rank_items.end());

        co_yield ::move(item_vector);
    }
}
