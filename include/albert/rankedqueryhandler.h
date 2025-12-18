// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/generatorqueryhandler.h>
#include <albert/rankitem.h>
#include <vector>

namespace albert
{

///
/// Usage-ranked query handler.
///
/// Convenience base class for triggered query handlers that return a complete set of match-scored
/// items eagerly. \ref rankItems is executed in a worker thread, allowing CPU-bound work without
/// blocking the main thread. The provided match scores will be combined with the usage-based
/// scoring weighted by user configuration. Finally the items will be yielded lazily in order of
/// their final score.
///
/// \ingroup util_query
///
class ALBERT_EXPORT RankedQueryHandler : public GeneratorQueryHandler
{
public:
    ///
    /// Returns a list of scored matches for _context_.
    ///
    /// The match score should make sense and often is the fraction of matched characters (legth of
    /// query string / length of matched string). The empty pattern matches everything and returns
    /// all items with a score of 0.
    ///
    /// \note Executed in a background thread.
    ///
    virtual std::vector<RankItem> rankItems(QueryContext &context) = 0;

    /// Yields _rank_items_ lazily sorted.
    static ItemGenerator lazySort(std::vector<RankItem> rank_items);

    /// Yields result of \ref rankItems for _context_ usage scored and lazily sorted.
    ItemGenerator items(QueryContext &context) override;

protected:
    /// Destructs the handler.
    ~RankedQueryHandler() override;
};

}  // namespace albert
