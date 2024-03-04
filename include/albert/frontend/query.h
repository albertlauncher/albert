// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QAbstractListModel>
#include <QObject>
#include <QString>

namespace albert
{

///
///  Interface class for queries used by frontends.
///
///  @see Frontend
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

    /// True if the query thread stopped.
    Q_INVOKABLE virtual bool isFinished() const = 0;

    /// True if query has not been cancelled.
    Q_INVOKABLE virtual const bool &isValid() const = 0;

    /// True if this query has a trigger.
    Q_INVOKABLE virtual bool isTriggered() const = 0;

    /// Returns the matches.
    Q_INVOKABLE virtual QAbstractListModel *matches() = 0;

    /// Returns the fallbacks.
    Q_INVOKABLE virtual QAbstractListModel *fallbacks() = 0;

    /// Executes match a match action.
    Q_INVOKABLE virtual void activateMatch(uint item, uint action = 0) = 0;

    /// Executes match a fallback action.
    Q_INVOKABLE virtual void activateFallback(uint item, uint action = 0) = 0;

protected:

    /// Private destructor. Lifetime is handled by the session.
    virtual ~Query() = default;

signals:

    /// Emitted when the query finished processing
    void finished();
};

}
