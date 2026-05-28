// SPDX-FileCopyrightText: 2026 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/queryhandler.h>
#include <albert/rankitem.h>
#include <memory>
#include <vector>
namespace QCoro { template<typename T> class Generator; }

namespace albert
{
class Item;

using ItemGenerator = QCoro::Generator<std::vector<std::shared_ptr<albert::Item>>>;

///
/// Coroutine-based synchronous generator query handler.
///
/// Convenience base class for implementing triggered query handlers using C++ coroutines. Results
/// are produced lazily via a synchronous item generator. Item production is executed in a worker
/// thread, allowing CPU-bound work without blocking the main thread. The items are displayed in the
/// order they are yielded.
///
/// If your query returns ordered items lazily, yield them directly. Note that the view will fetch
/// items as long as the view is not filled. To avoid UI flicker introduced by multiple sequential
/// fetches try to yield chunks of tens.
///
/// If your query returns ordered items eagerly and the amount of items is non negliable split and
/// yield them in chunks for performance.
///
/// If your query returns unordered, scored items eagerly use \ref lazySort(std::vector<RankItem>)
/// to return a generator that yields them lazily sorted. If you want your items to be sorted by
/// usage apply the \ref QueryContext::usageScoring before passing them to \ref
/// lazySort(std::vector<RankItem>) or use \ref lazySort(std::vector<RankItem>, const UsageScoring&).
///
/// This class is intended for computational workloads. For I/O-bound or event-driven tasks, prefer
/// \ref AsyncGeneratorQueryHandler.
///
/// \ingroup util_query
///
class ALBERT_EXPORT GeneratorQueryHandler : public QueryHandler
{
public:
    ///
    /// Yields batches of items for _context_ lazily.
    ///
    /// The batch size is defined by the implementation.
    ///
    /// \note Executed in a background thread.
    ///
    /// \note GCC-13 does not support returning temporary values in generators.
    ///       So for as long as Ubuntu 24.04 is supported, we have to return lvalues.
    ///
    virtual ItemGenerator items(QueryContext &context) = 0;

    /// Returns a generator yielding _rank_items_ lazily sorted by score.
    static ItemGenerator lazySort(std::vector<RankItem> rank_items);

    /// Applies _usage_scoring_ to _rank_items_ and returns a generator yielding lazily sorted.
    ItemGenerator lazySort(std::vector<RankItem> rank_items,
                           const UsageScoring &usage_scoring) const;

protected:
    /// Destructs the handler.
    ~GeneratorQueryHandler() override;

    /// Returns a threaded synchronous generator query execution for _context_.
    std::unique_ptr<QueryExecution> execution(QueryContext &context) override;
};
}  // namespace albert
