// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <albert/item.h>
#include <albert/querycontext.h>
#include <memory>
#include <ranges>
#include <vector>

namespace albert
{
class Extension;

///
/// Result item associating an item with an extension.
///
/// \ingroup core_query
///
class ALBERT_EXPORT QueryResult
{
public:
    const Extension *extension; ///< The extension providing the item.
    std::shared_ptr<Item> item; ///< The item.
};

///
/// Query results container.
///
/// Holds the results of a \ref Query and emits signals as expected by the UI models.
///
/// \ingroup core_query
///
class ALBERT_EXPORT QueryResults : public QObject, private Item::Observer
{
    Q_OBJECT

public:

    /// Constructs query results with the _context_ it belongs to.
    QueryResults(const QueryContext &context);

    /// Destructs the query results.
    ~QueryResults() override;

    /// Returns the result at index _index_.
    inline QueryResult &operator[](size_t index) { return results[index]; }

    /// @copybrief operator[](size_t)
    inline const QueryResult &operator[](size_t index) const { return results[index]; }

    /// Returns the number of results.
    inline uint count() const { return results.size(); }

    /// Activates the action at _action_index_ of the result item at _item_index_.
    bool activate(uint item_index, uint action_index = 0);

    ///
    /// Appends a \ref QueryResult constructed from _extension_ and _item_.
    ///
    /// Use the range add methods to avoid UI flicker.
    ///
    void add(const Extension &extension, ItemPtr auto &&item)
    {
        emit resultsAboutToBeInserted(results.size(), results.size());
        item->addObserver(this);
        results.emplace_back(&extension, std::forward<decltype(item)>(item));
        emit resultsInserted();
    }

    ///
    /// Appends a \ref QueryResult constructed from _item_ and the handler this results belong to.
    ///
    /// Use the range add methods to avoid UI flicker.
    ///
    void add(ItemPtr auto &&item) { add(context.handler(), std::forward<decltype(item)>(item)); }

    /// Appends _query_results_ to the results.
    void add(std::ranges::range auto &&query_results)
        requires(std::same_as<QueryResult, std::ranges::range_value_t<decltype(query_results)>>)
    {
        if (!query_results.empty())
        {
            const auto count = std::ranges::distance(query_results);
            results.reserve(results.size() + count);
            emit resultsAboutToBeInserted(results.size(), results.size() + count - 1);
            for (auto&& query_result : query_results)
            {
                query_result.item->addObserver(this);
                // results.emplace_back(std::forward_like<decltype(query_results)>(query_result));
                // TODO remove if forward_like is available everywhere (26.04)
                if constexpr (std::is_lvalue_reference_v<decltype(query_results)>)
                    results.emplace_back(query_result);
                else
                    results.emplace_back(std::move(query_result));
            }
            emit resultsInserted();
        }
    }

    /// Appends \ref QueryResult's constructed from _extension_ and _items_.
    void add(const Extension &extension, ItemRange auto &&items)
    {
        if (!items.empty())
        {
            const auto count = std::ranges::distance(items);
            results.reserve(results.size() + count);
            emit resultsAboutToBeInserted(results.size(), results.size() + count - 1);
            for (auto&& item : items)
            {
                item->addObserver(this);
                // results.emplace_back(std::forward_like<decltype(query_results)>(query_result));
                // TODO remove if forward_like is available everywhere (26.04)
                if constexpr (std::is_lvalue_reference_v<decltype(items)>)
                    results.emplace_back(&extension, item);
                else
                    results.emplace_back(&extension, std::move(item));
            }
            emit resultsInserted();
        }
    }

    /// Appends \ref QueryResult's constructed from _items_ and the handler this results belong to.
    void add(ItemRange auto &&items){ add(context.handler(), std::forward<decltype(items)>(items)); }

    /// Removes _count_ results starting from _index_.
    void remove(uint index, uint count = 1)
    {
        for (auto i = index; i < index + count; ++i)
            results[i].item->removeObserver(this);

        emit resultsAboutToBeRemoved(index, index + count - 1);
        results.erase(results.begin() + index,results.begin() + index + count);
        emit resultsRemoved();

    }

    /// Removes all results.
    void reset()
    {
        for (auto&& result : results)
            result.item->removeObserver(this);
        emit resultsAboutToBeReset();
        results.clear();
        emit resultsReset();
    }

signals:

    /// Emitted before results are inserted.
    void resultsAboutToBeInserted(int first, int last);

    /// Emitted after results have been inserted.
    void resultsInserted();

    /// Emitted before results are removed.
    void resultsAboutToBeRemoved(int first, int last);

    /// Emitted after results have been removed.
    void resultsRemoved();

    /// Emitted before results are moved.
    void resultsAboutToBeMoved(int srcFirst, int srcLast, int dst);

    /// Emitted after results have been moved.
    void resultsMoved();

    /// Emitted before all results are reset.
    void resultsAboutToBeReset();

    /// Emitted after all results have been reset.
    void resultsReset();

    /// Emitted when a result changed.
    void resultChanged(uint i);

    /// Emitted when a result was activated.
    void resultActivated(QString query, QString extension_id, QString item_id, QString action_id);

private:

    void notify(const albert::Item *item) override;

    const QueryContext &context;
    std::vector<QueryResult> results;

};

}
