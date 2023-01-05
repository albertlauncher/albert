// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/globalqueryhandler.h"
#include <set>

struct GlobalSearch : public albert::QueryHandler
{
    std::set<albert::GlobalQueryHandler*> handlers;
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void handleQuery(Query &query) const override;
};
