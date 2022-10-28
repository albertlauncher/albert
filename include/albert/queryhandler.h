// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include "query.h"
#include <QString>

namespace albert
{
/// Trigger only handler. Use this for realtime long running queries
struct ALBERT_EXPORT QueryHandler : virtual public Extension
{
    virtual void handleQuery(albert::Query &query) const = 0;  /// Called on triggered query.
    virtual QString synopsis() const;  /// The synopsis, displayed on empty query. Default empty
    virtual QString default_trigger() const;  /// The default (not user defined) trigger. Default Extension::id().
    virtual bool allow_trigger_remap() const;  /// Enable user remapping of the trigger. Default false.
};
}


