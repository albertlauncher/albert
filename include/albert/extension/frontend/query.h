// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QObject>
#include <QString>
#include <memory>
class QAbstractListModel;

namespace albert
{
class Item;

///
class ALBERT_EXPORT Query : public QObject
{
public:
    virtual const QString &synopsis() const = 0;  ///< The trigger of this query.
    virtual const QString &trigger() const = 0;  ///< The trigger of this query
    virtual const QString &string() const = 0;  ///< Query string _excluding_ the trigger.

    virtual void run() = 0;  ///< Asynchronous query processing done.
    virtual void cancel() = 0;  ///< Call this if you dont need the query anymore
    virtual bool isFinished() const = 0;  ///< True if the query thread stopped
    virtual bool isValid() const = 0;  ///< True if query has not been cancelled

    virtual QAbstractListModel &matches() = 0;  ///< You borrow
    virtual QAbstractListModel &fallbacks() = 0;  ///< You borrow
    virtual QAbstractListModel *matchActions(uint item) const = 0;  ///< You take ownership
    virtual QAbstractListModel *fallbackActions(uint item) const = 0;  ///< You take ownership
    virtual void activateMatch(uint item, uint action) = 0;
    virtual void activateFallback(uint item, uint action) = 0;

    virtual void add(const std::shared_ptr<Item> &item) = 0;  ///< Copy add item
    virtual void add(std::shared_ptr<Item> &&item) = 0;  ///< Move add item
    virtual void add(const std::vector<std::shared_ptr<Item>> &items) = 0;  ///< Copy add items
    virtual void add(std::vector<std::shared_ptr<Item>> &&items) = 0;  ///< Move add items

Q_OBJECT signals:
    void finished();
};

}
