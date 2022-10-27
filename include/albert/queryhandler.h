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
    virtual void handleQuery(albert::Query &query) const = 0;  /// Called on triggered query
    virtual QString synopsis() const { return {}; }  /// The synopsis, displayed on empty query
    virtual QString default_trigger() const { return id(); }  /// The default (not user defined) trigger
    virtual bool allow_trigger_remap() const { return true; }  /// Enable user remapping of the trigger
};

/// Global search handler. Do not use this for long running queries
struct ALBERT_EXPORT GlobalQueryHandler : public QueryHandler
{
    virtual std::vector<Match> rankedItems(const albert::Query &query) const = 0;  /// Called on global search
    void handleQuery(albert::Query &query) const override;  /// Sorts items returned by rankedItems
};

///
struct ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
    IndexQueryHandler();
    ~IndexQueryHandler();

    virtual std::map<albert::SharedItem,std::map<QString,albert::Score>> indexItems() const = 0;  // Item factory

    std::vector<Match> rankedItems(const albert::Query &query) const override;  /// Queries index
    QString synopsis() const override;  /// <filter>

    void updateIndex();  /// Call this when your items changed

    struct Private;
    Private *d;
};

}


