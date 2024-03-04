// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include <QString>
#include <memory>
class TriggerQueryHandlerPrivate;
class QueryEngine;

namespace albert
{
class Item;

/// Triggered query handler class.
/// If the trigger matches this handler is the only query handler chosen to
/// process the user query. Inherit this class if you dont want your results to
/// be reordered or if you want to display your items of a long running query
/// as soon as they are available.
class ALBERT_EXPORT TriggerQueryHandler : virtual public Extension
{
public:
    TriggerQueryHandler();
    ~TriggerQueryHandler();

    /// The user configured trigger of this handler.
    QString trigger() const;

    /// The synopsis, displayed on empty query.
    /// Use this to give the user hints about accepted query strings.
    /// Default empty.
    virtual QString synopsis() const;

    /// The default (not user defined) trigger. Default Extension::id().
    virtual QString defaultTrigger() const;

    /// Enable user remapping of the trigger. Default false.
    virtual bool allowTriggerRemap() const;

    /// Fuzzy matching capability. Default false.
    virtual bool supportsFuzzyMatching() const;

    /// Fuzzy matching. Default false.
    virtual bool fuzzyMatching() const;

    /// Fuzzy matching behavior. Default does nothing.
    virtual void setFuzzyMatching(bool enabled);

    /// The query interface used by TriggerQueryHandler
    /// @see handleTriggerQuery
    class ALBERT_EXPORT TriggerQuery
    {
    public:
        virtual ~TriggerQuery() = default;

        /// The trigger of this query if any.
        virtual QString trigger() const = 0;

        /// The query string excluding the trigger.
        virtual QString string() const = 0;

        /// True if query has not been cancelled.
        /// @note Stop query processing if false.
        virtual const bool &isValid() const = 0;

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
    };

    /// The trigger query processing function.
    /// @note Executed in a worker thread.
    virtual void handleTriggerQuery(TriggerQuery*) const = 0;

private:
    std::unique_ptr<TriggerQueryHandlerPrivate> d;
    friend class ::QueryEngine;
};

}
