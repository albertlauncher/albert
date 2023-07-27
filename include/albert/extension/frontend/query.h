// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QObject>
#include <QString>
#include <QAbstractListModel>
#include <memory>
class QAbstractListModel;

namespace albert
{
class Item;

/// Interface class for queries used by frontends
/// @see Frontend
class ALBERT_EXPORT Query : public QObject
{
    Q_OBJECT
public:
    virtual ~Query() = default;

    Q_INVOKABLE virtual QString synopsis() const = 0;  ///< The trigger of this query.
    Q_INVOKABLE virtual QString trigger() const = 0;  ///< The trigger of this query
    Q_INVOKABLE virtual QString string() const = 0;  ///< Query string _excluding_ the trigger.

    Q_INVOKABLE virtual void run() = 0;  ///< Asynchronous query processing done.
    Q_INVOKABLE virtual void cancel() = 0;  ///< Call this if you dont need the query anymore.
    Q_INVOKABLE virtual bool isFinished() const = 0;  ///< True if the query thread stopped.
    Q_INVOKABLE virtual const bool &isValid() const = 0;  ///< True if query has not been cancelled.
    Q_INVOKABLE virtual bool isTriggered() const = 0;  ///< True if this query has a triggert.

    Q_INVOKABLE virtual QAbstractListModel *matches() = 0;  ///< You borrow
    Q_INVOKABLE virtual QAbstractListModel *fallbacks() = 0;  ///< You borrow
    Q_INVOKABLE virtual QAbstractListModel *matchActions(uint item) const = 0;  ///< You take ownership
    Q_INVOKABLE virtual QAbstractListModel *fallbackActions(uint item) const = 0;  ///< You take ownership
    Q_INVOKABLE virtual void activateMatch(uint item, uint action = 0) = 0; ///< Executes match a match action
    Q_INVOKABLE virtual void activateFallback(uint item, uint action = 0) = 0; ///< Executes match a fallback action

signals:
    void finished(); ///< Emitted when the query finished processing
};

}
