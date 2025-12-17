// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/generatorqueryhandler.h>
#include <albert/rankitem.h>
#include <vector>

namespace albert
{

///
/// Abstract trigger query handler extension.
///
/// If the trigger matches, this handler is the only query handler chosen to
/// process the user query. Inherit this class if you dont want your results to
/// be reordered or if you want to display your items of a long running query
/// as soon as they are available.
///
/// \ingroup util_query
///
class ALBERT_EXPORT ThreadedQueryHandler : public GeneratorQueryHandler
{
public:
    ///
    /// Returns scored items matching the _query_.
    ///
    /// The match score should make sense and often is the fraction of matched characters (legth of
    /// query string / length of matched string).
    ///
    /// Note that the empty pattern matches everything and returns all items with a score of 0.
    ///
    /// @note Executed in a worker thread.
    ///
    virtual std::vector<RankItem> rankItems(Query &query) = 0;

    /// Yields items from _rank_items_ lazily sorted by score.
    static ItemGenerator lazySort(std::vector<RankItem> rank_items);

    /// Yields items for _query_ lazily sorted taking usage scoring into account.
    ItemGenerator items(Query &query) override;

protected:
    /// Destructs the handler.
    ~ThreadedQueryHandler() override;
};

}  // namespace albert
