// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

/// \defgroup query_util Query utility API
/// \ingroup util
/// Utility classes and helper functions for query related tasks.

#pragma once
#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <albert/export.h>
#include <memory>
#include <vector>

namespace albert
{
class Item;
class Extension;

class ALBERT_EXPORT ResultItem
{
public:
    const Extension &extension;
    std::shared_ptr<Item> item;
};

///
/// Common query object.
///
/// \ingroup core
///
class ALBERT_EXPORT Query : public QObject
{
    Q_OBJECT

public:

    ///
    /// Returns the synopsis of this query.
    ///
    Q_INVOKABLE virtual QString synopsis() const = 0;

    ///
    /// Returns the trigger of this query.
    ///
    Q_INVOKABLE virtual QString trigger() const = 0;

    ///
    /// Returns the query string excluding the trigger.
    ///
    Q_INVOKABLE virtual QString string() const = 0;

    ///
    /// Returns `true` if the query is being processed; otherwise returns `false`.
    ///
    Q_INVOKABLE virtual bool isActive() const = 0;

    ///
    /// Returns `false` if the query has been cancelled; otherwise returns `true`.
    ///
    Q_INVOKABLE virtual const bool &isValid() const = 0;

    ///
    /// Returns `true` if the query has a trigger; otherwise returns `false`.
    ///
    Q_INVOKABLE virtual bool isTriggered() const = 0;

    ///
    /// Returns the matches.
    ///
    Q_INVOKABLE virtual const std::vector<ResultItem> &matches() = 0;

    ///
    /// Returns the fallbacks.
    ///
    Q_INVOKABLE virtual const std::vector<ResultItem> &fallbacks() = 0;

    ///
    /// Activates the action at _action_index_ of match item at _item_index_.
    ///
    Q_INVOKABLE virtual bool activateMatch(uint item_index, uint action_index = 0) = 0;

    ///
    /// Activates the action at _action_index_ of fallback item at _item_index_.
    ///
    Q_INVOKABLE virtual bool activateFallback(uint item_index, uint action_index = 0) = 0;

    ///
    /// Adds _item_ to the query results.
    ///
    /// Use batch methods to avoid expensive locking and UI flicker.
    ///
    virtual void add(const std::shared_ptr<Item> &item) = 0;

    ///
    /// Adds _item_ to the query results using move semantics.
    ///
    /// @copydetails add(const std::shared_ptr<Item> &item)
    ///
    virtual void add(std::shared_ptr<Item> &&item) = 0;

    ///
    /// Adds _items_ to the query results.
    ///
    virtual void add(const std::vector<std::shared_ptr<Item>> &items) = 0;

    ///
    /// Adds _items_ to the query results using move semantics.
    ///
    virtual void add(std::vector<std::shared_ptr<Item>> &&items) = 0;

    ///
    /// Converts the query to a `QString`.
    ///
    /// Syntactic sugar for implicit context conversions.
    ///
    inline operator QString() const { return string(); }

protected:

    ///
    /// Destructs the query.
    ///
    ~Query() override;

signals:

    ///
    /// Emitted before `count` matches are added to the matches.
    ///
    void matchesAboutToBeAdded(uint count);

    ///
    /// Emitted after matches have been added to the matches.
    ///
    void matchesAdded();

    ///
    /// Emitted when the query has been invalidated.
    ///
    void invalidated();

    ///
    /// Emitted when query processing started or finished.
    ///
    void activeChanged(bool active);

    ///
    /// Emitted when a match item changed any of its fields
    ///
    void dataChanged(uint i);
};

}
