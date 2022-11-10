// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include <QString>

namespace albert
{
class Item;

/// Trigger only handler. Use this for realtime long running queries
struct ALBERT_EXPORT QueryHandler : virtual public Extension
{
    struct Query{
        virtual ~Query() = default;
        virtual const QString &trigger() const = 0;  /// The trigger of this query.
        virtual const QString &string() const = 0;  /// Query string _excluding_ the trigger.
        virtual bool isValid() const = 0;  /// True if query has not been cancelled.
        virtual void add(const std::shared_ptr<Item> &item) = 0;  /// Copy add item
        virtual void add(std::shared_ptr<Item> &&item) = 0;  /// Move add item
        virtual void add(std::vector<std::shared_ptr<Item>> &&items) = 0;  /// Move add items
    };

    virtual void handleQuery(Query &query) const = 0;  /// Called on triggered query.
    virtual QString synopsis() const;  /// The synopsis, displayed on empty query. Default empty.
    virtual QString default_trigger() const;  /// The default (not user defined) trigger. Default Extension::id().
    virtual bool allow_trigger_remap() const;  /// Enable user remapping of the trigger. Default false.
};
}


