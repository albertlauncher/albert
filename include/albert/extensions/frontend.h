// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "../extension.h"
#include <QObject>
#include <memory>
class QAbstractListModel;
class QWidget;
class QueryEngine;

namespace albert
{

/// Convention on the item roles passed around
enum class ALBERT_EXPORT ItemRoles
{
    TextRole = Qt::DisplayRole,  // QString
    IconRole = Qt::DecorationRole,  // QIcon for QWidgets
    SubTextRole = Qt::UserRole,  // QString
    IconPathRole,  // QString for QML
    IconUrlsRole,  // QStringList
    InputActionRole  // QString
};


class ALBERT_EXPORT Query : public QObject
{
public:
    virtual const QString &synopsis() const = 0;  ///< The trigger of this query.
    virtual const QString &trigger() const = 0;  ///< The trigger of this query.
    virtual const QString &string() const = 0;  ///< Query string _excluding_ the trigger.

    virtual void run() = 0;  ///< Asynchronous query processing done.
    virtual void cancel() = 0;  ///< Call this if you dont need the query anymore
    virtual bool isFinished() const = 0;  ///< True if the query thread stopped

    virtual QAbstractListModel &matches() = 0;  ///< You borrow
    virtual QAbstractListModel &fallbacks() = 0;  ///< You borrow
    virtual QAbstractListModel *matchActions(uint item) const = 0;  ///< You take ownership
    virtual QAbstractListModel *fallbackActions(uint item) const = 0;  ///< You take ownership
    virtual void activateMatch(uint item, uint action) = 0;
    virtual void activateFallback(uint item, uint action) = 0;

Q_OBJECT signals:
    void finished();
};


/// The interface for albert frontends
class ALBERT_EXPORT Frontend : virtual public albert::Extension
{
public:
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;
    virtual QString input() const = 0;
    virtual void setInput(const QString&) = 0;
    virtual QWidget *createFrontendConfigWidget() = 0;
    void setEngine(QueryEngine*);
protected:
    std::shared_ptr<Query> query(const QString &query) const;
private:
    QueryEngine *query_engine;
};

}
