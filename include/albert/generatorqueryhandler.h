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
/// C++ coroutine based, threaded item generator query handler.
///
/// Convenience class for lazy, CPU-bound tasks.
///
/// CAUTION: The items of the provided generator are implicitly yielded in a background thread.
/// Make sure your data is handled thread safe.
///
/// Note: `std::generator` is not available on all major platforms yet. Therefore this class is
/// based on `QCoro::Generator` for now.
///
/// \ingroup util_query
///
class ALBERT_EXPORT GeneratorQueryHandler : public QueryHandler
{
public:
    ///
    /// Yields batches of items for _query_ lazily.
    ///
    /// The batch size is defined by the implementation.
    ///
    /// \note Executed in a background thread.
    ///
    virtual ItemGenerator items(Query &query) = 0;

protected:
    /// Destructs the handler.
    ~GeneratorQueryHandler() override;

    /// Returns a threaded synchronous generator query execution.
    std::unique_ptr<QueryExecution> execution(Query &query) override;
};
}  // namespace albert
