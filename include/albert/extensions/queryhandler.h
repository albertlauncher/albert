// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "../extension.h"
#include <QStringList>
#include <functional>
#include <memory>
#include <vector>

namespace albert
{

class ALBERT_EXPORT Action  /// Action for items
{
public:
    Action(QString id, QString text, std::function<void()> function);
    QString id;  /// Identifier of the action
    QString text;  /// The description of the action
    std::function<void()> function;  /// Activates the item
};


class ALBERT_EXPORT Item  /// Items displayed in the query results list
{
public:
    virtual ~Item() = default;
    virtual QString id() const = 0;  /// Per extension unique identifier
    virtual QString text() const = 0;  /// The title for the item
    virtual QString subtext() const = 0;  /// The descriptive subtext
    /// URLs for the icon provider
    /// 'xdg:<icon-name>' performs freedesktop icon theme specification lookup (linux only).
    /// 'qfip:<path>' uses QFileIconProvider to get the icon for thefile.
    /// ':<path>' is a QResource path.
    /// '<path>' is interpreted as path to a local image file.
    /// @note Icons are cached.
    virtual QStringList iconUrls() const = 0;
    virtual QString inputActionText() const;  /// Input replacement for input action (usually Tab)
    virtual bool hasActions() const;  /// Indicates that the item has actions
    virtual std::vector<Action> actions() const;  /// The list of actions, this item provides
};


/// Exclusive/Triggered only query handler.
/// Use this if you want to control the order of the items or the query
/// takes a lot of time(for e.g. long runnig tasks, online searches, …)
class ALBERT_EXPORT QueryHandler : virtual public Extension
{
public:
    class Query;
    virtual void handleQuery(Query &query) const = 0;  /// Called on triggered query.
    virtual std::vector<std::shared_ptr<Item>> fallbacks(const QString &) const;  /// Fallbacks provided by this handler
    virtual QString synopsis() const;  /// The synopsis, displayed on empty query. Default empty.
    virtual QString default_trigger() const;  /// The default (not user defined) trigger. Default Extension::id().
    virtual bool allow_trigger_remap() const;  /// Enable user remapping of the trigger. Default false.
};


class ALBERT_EXPORT QueryHandler::Query
{
public:
    virtual ~Query() = default;
    virtual const QString &trigger() const = 0;  /// The trigger of this query.
    virtual const QString &string() const = 0;  /// Query string _excluding_ the trigger.
    virtual bool isValid() const = 0;  /// True if query has not been cancelled.
    virtual void add(const std::shared_ptr<Item> &item) = 0;  /// Copy add item
    virtual void add(std::shared_ptr<Item> &&item) = 0;  /// Move add item
    virtual void add(const std::vector<std::shared_ptr<Item>> &items) = 0;  /// Copy add items
    virtual void add(std::vector<std::shared_ptr<Item>> &&items) = 0;  /// Move add items
};

}
