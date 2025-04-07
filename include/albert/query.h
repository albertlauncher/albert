// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

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
class ALBERT_EXPORT Query : public QObject
{
    Q_OBJECT

public:

    /// The synopsis of this query.
    Q_INVOKABLE virtual QString synopsis() const = 0;

    /// The trigger of this query.
    Q_INVOKABLE virtual QString trigger() const = 0;

    /// Query string _excluding_ the trigger.
    Q_INVOKABLE virtual QString string() const = 0;

    /// True if the query is currently being processed.
    Q_INVOKABLE virtual bool isActive() const = 0;

    /// True if the query thread stopped.
    Q_INVOKABLE virtual bool isFinished() const = 0;

    /// True if query has not been cancelled.
    Q_INVOKABLE virtual const bool &isValid() const = 0;

    /// True if this query has a trigger.
    Q_INVOKABLE virtual bool isTriggered() const = 0;

    /// Returns the matches.
    Q_INVOKABLE virtual const std::vector<ResultItem> &matches() = 0;

    /// Returns the fallbacks.
    Q_INVOKABLE virtual const std::vector<ResultItem> &fallbacks() = 0;

    /// Executes match a match action.
    Q_INVOKABLE virtual bool activateMatch(uint item, uint action = 0) = 0;

    /// Executes match a fallback action.
    Q_INVOKABLE virtual bool activateFallback(uint item, uint action = 0) = 0;

    /// Copy add single item.
    /// @note Use batch add if you can to avoid UI flicker.
    /// @see add(const std::vector<std::shared_ptr<Item>> &items)
    virtual void add(const std::shared_ptr<Item> &item) = 0;

    /// Move add single item.
    /// @note Use batch add if you can to avoid UI flicker.
    /// @see add(std::vector<std::shared_ptr<Item>> &&items)
    virtual void add(std::shared_ptr<Item> &&item) = 0;

    /// Copy add multiple items.
    virtual void add(const std::vector<std::shared_ptr<Item>> &items) = 0;

    /// Move add multiple items.
    virtual void add(std::vector<std::shared_ptr<Item>> &&items) = 0;

    /// Type conversion to QString
    /// Syntactic sugar for context conversions
    /// @since 0.24
    inline operator QString() const { return string(); }

protected:

    ~Query() override;

signals:

    /// Emitted before `count` matches are added to the matches.
    void matchesAboutToBeAdded(uint count);

    /// Emitted after matches have been added to the matches.
    void matchesAdded();

    /// Emitted when the query has been invalidated.
    void invalidated();

    /// Emitted when query processing started or finished.
    void activeChanged(bool active);

    /// Emitted when a match item changed any of its fields
    void dataChanged(uint i);
};

}
