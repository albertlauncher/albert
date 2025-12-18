// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/queryhandler.h>
#include <memory>
#include <vector>
namespace QCoro { template<typename T> class AsyncGenerator; }

namespace albert
{
class Item;

using AsyncItemGenerator = QCoro::AsyncGenerator<std::vector<std::shared_ptr<albert::Item>>>;

///
/// Coroutine-based asynchronous generator query handler.
///
/// Convenience base class for implementing triggered query handlers using C++ coroutines. Results
/// are produced lazily via an asynchronous item generator. The items are displayed in the order
/// they are yielded.
///
/// This class is suitable for I/O-bound query handling (e.g. network requests, subprocessing,
/// etc.). For CPU-bound work, prefer \ref GeneratorQueryHandler or its subclasses.
///
/// If you derive this class you want to link against QCoro which provides coroutine support for Qt
/// classes. Note that QCoro is still in development.
///
/// \ingroup util_query
///
class ALBERT_EXPORT AsyncGeneratorQueryHandler : public QueryHandler
{
public:
    ///
    /// Yields batches of items for _context_ asynchronously and lazily.
    ///
    /// The batch size is defined by the implementation.
    ///
    /// \note Called from main thread. Do not run blocking operations in it.
    ///
    virtual AsyncItemGenerator items(QueryContext &context) = 0;

protected:
    /// Destructs the handler.
    ~AsyncGeneratorQueryHandler() override;

    /// Returns an asynchronous generator query execution for _context_.
    std::unique_ptr<QueryExecution> execution(QueryContext &context) override;
};
}  // namespace albert
