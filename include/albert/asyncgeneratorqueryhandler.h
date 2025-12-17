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
/// C++ coroutine based, asynchronous item generator query handler.
///
/// This class is especially useful if your task is I/O bound (e.g. network requests, subprocessing,
/// etc.). If your task is CPU bound consider using \ref GeneratorQueryHandler or its subclasses.
///
/// Note that \ref items is called from the main thread. Do not run blocking operations in it.
///
/// If you derive this class you want to link against QCoro which provides coroutine support for Qt
/// classes. Note that QCoro is still in development and as such does not have a stable API/ABI.
/// Also some of its awaiters introduce memory leaks and segfaults so maybe it is better to not use
/// system packaged QCoro and carefully test your implementation.
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
    /// \note Called from main thread.
    ///
    virtual AsyncItemGenerator items(QueryContext &context) = 0;

protected:
    /// Destructs the handler.
    ~AsyncGeneratorQueryHandler() override;

    /// Returns an asynchronous generator query execution for _context_.
    std::unique_ptr<QueryExecution> execution(QueryContext &context) override;
};
}  // namespace albert
