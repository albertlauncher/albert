// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/item.h>
#include <albert/query.h>
#include <albert/queryhandler.h>
#include <memory>
#include <mutex>
#include <vector>

namespace albert
{
class RankItem;


class ThreadedQuery : public albert::Query
{
public:
    /// Adds _item_ to the query results thread-safe.
    void add(ItemPtr auto &&item)
    {
        if (const auto lock = getLock();
            isValid())
            matches().emplace_back(std::forward<decltype(item)>(item));
        collect();
    }

    /// Adds _items_ to the query results thread-safe.
    void add(ItemRange auto &&items)
    {
        if (const auto lock = getLock();
            isValid() || !items.empty())
        {
            auto &m = matches();
            m.reserve(m.size() + std::ranges::distance(items));
            for (auto &&item : items)
                // m.emplace_back(std::forward_like<decltype(items)>(item));
                // TODO remove if forward_like is available everywhere (26.04)
                if constexpr (std::is_lvalue_reference_v<decltype(items)>)
                    m.emplace_back(item);           // copy if lvalue range
                else
                    m.emplace_back(std::move(item)); // move if rvalue range
        }
        collect();
    }

private:
    virtual std::lock_guard<std::mutex> getLock() = 0;
    virtual std::vector<std::shared_ptr<Item>> &matches() = 0;
    virtual void collect() = 0;
};

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
class ALBERT_EXPORT ThreadedQueryHandler : public QueryHandler
{
public:

    ///
    /// Handles the _threaded_query_.
    ///
    /// @note Executed in a thread.
    ///
    virtual void handleThreadedQuery(ThreadedQuery &threaded_query) = 0;

protected:

    std::unique_ptr<QueryExecution> execution(Query &query) override;

};

}  // namespace albert
