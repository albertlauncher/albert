// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/queryhandler.h>
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
    virtual ItemGenerator items(QueryContext &context) = 0;

protected:
    /// Destructs the handler.
    ~GeneratorQueryHandler() override;

    /// Returns a threaded synchronous generator query execution for _context_.
    std::unique_ptr<QueryExecution> execution(QueryContext &context) override;
};
}  // namespace albert
