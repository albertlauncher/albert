// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "extensions/globalqueryhandlerprivate.h"
#include <set>

class GlobalSearch final : public albert::QueryHandler
{
public:
    std::set<GlobalQueryHandlerPrivate*> handlers;
private:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void handleQuery(Query &query) const override;
};
