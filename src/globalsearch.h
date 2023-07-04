// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include <set>
namespace albert { class GlobalQueryHandler; }

class GlobalSearch final : public albert::TriggerQueryHandler
{
public:
    GlobalSearch(const std::set<albert::GlobalQueryHandler*> &handlers);
private:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void handleTriggerQuery(TriggerQuery *query) const override;

    const std::set<albert::GlobalQueryHandler*> &handlers;
};
