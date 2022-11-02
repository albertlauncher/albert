// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "globalqueryhandler.h"

namespace albert
{
/// Provides an indexed item search
struct ALBERT_EXPORT IndexQueryHandler : public GlobalQueryHandler
{
    IndexQueryHandler();
    ~IndexQueryHandler() override;

    virtual std::map<std::shared_ptr<Item>,std::map<QString,uint16_t>> indexItems() const = 0;  // Item factory

    std::vector<std::pair<std::shared_ptr<albert::Item>,uint16_t>> rankedItems(const albert::Query &query) const override final;  /// Queries index
    QString synopsis() const override;  /// Default <filter>

    void updateIndex();  /// Call this when your items changed

    struct Private;
    Private *d;
};
}
