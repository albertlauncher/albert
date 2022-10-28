// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "globalqueryhandler.h"

namespace albert
{
/// Provides an indexed item search
struct ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
    IndexQueryHandler();
    ~IndexQueryHandler();

    virtual std::map<albert::SharedItem,std::map<QString,albert::Score>> indexItems() const = 0;  // Item factory

    std::vector<Match> rankedItems(const albert::Query &query) const override final;  /// Queries index
    QString synopsis() const override;  /// Default <filter>

    void updateIndex();  /// Call this when your items changed

    struct Private;
    Private *d;
};
}
