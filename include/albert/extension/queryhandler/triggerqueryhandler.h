// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include <QString>

namespace albert
{
class Item;

/// Exclusive/Triggered only query handler.
/// Use this if you dont want your results to be rearranged
/// or if the query takes too much time to be in the global search.
class ALBERT_EXPORT TriggerQueryHandler : virtual public Extension
{
public:
    virtual QString synopsis() const;  ///< The synopsis, displayed on empty query. Default empty.
    virtual QString defaultTrigger() const;  ///< The default (not user defined) trigger. Default Extension::id().
    virtual bool allowTriggerRemap() const;  ///< Enable user remapping of the trigger. Default false.

    struct TriggerQuery
    {
        virtual ~TriggerQuery();
        virtual const QString &trigger() const = 0;  ///< The trigger of this query
        virtual const QString &string() const = 0;  ///< The query string
        virtual bool isValid() const = 0;  ///< True if query has not been cancelled
        virtual void add(const std::shared_ptr<Item> &item) = 0;  ///< Copy add item
        virtual void add(std::shared_ptr<Item> &&item) = 0;  ///< Move add item
        virtual void add(const std::vector<std::shared_ptr<Item>> &items) = 0;  ///< Copy add items
        virtual void add(std::vector<std::shared_ptr<Item>> &&items) = 0;  ///< Move add items
    };

    virtual void handleTriggerQuery(TriggerQuery*) const = 0;  ///< Called on triggered query.
};

}
