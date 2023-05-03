// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "extensions/globalqueryhandlerprivate.h"
#include <set>

class GlobalSearch final : public albert::TriggerQueryHandler
{
public:
    std::set<GlobalQueryHandlerPrivate*> handlers;
private:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void handleTriggerQuery(TriggerQuery &query) const override;
};
