// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include "albert/query.h"
#include <QString>

namespace albert
{
class Item;

///
/// Triggered query handler class.
///
/// If the trigger matches this handler is the only query handler chosen to
/// process the user query. Inherit this class if you dont want your results to
/// be reordered or if you want to display your items of a long running query
/// as soon as they are available.
///
class ALBERT_EXPORT TriggerQueryHandler : virtual public Extension
{
public:
    /// The synopsis, displayed on empty query.
    /// Use this to give the user hints about accepted query strings.
    /// Default empty.
    virtual QString synopsis() const;

    /// Enable user remapping of the trigger.
    /// Default false.
    virtual bool allowTriggerRemap() const;

    /// The default (not user defined) trigger.
    /// Default Extension::id().
    virtual QString defaultTrigger() const;

    /// Setter for the user defined trigger.
    /// Reimplement this if you need this information.
    /// Default does nothing.
    /// \since 0.24
    virtual void setTrigger(const QString &);

    /// Fuzzy matching capability.
    /// Default false.
    virtual bool supportsFuzzyMatching() const;

    /// Fuzzy matching behavior.
    /// Default does nothing.
    virtual void setFuzzyMatching(bool enabled);

    /// The trigger query processing function.
    /// @note Executed in a worker thread.
    virtual void handleTriggerQuery(Query*) = 0;

};

}
