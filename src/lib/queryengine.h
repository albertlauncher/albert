// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "extensionwatcher.h"
#include "fallbackprovider.h"
#include "query.h"
#include "queryhandler.h"
#include <map>
#include <memory>
#include <vector>

class QueryEngine : public albert::ExtensionWatcher<albert::QueryHandler>,
                    public albert::ExtensionWatcher<albert::FallbackProvider>
{
public:
    /// NOTE You borrow! Watch destroyed signal! (Happens on unloading plugins)
    albert::Query* query(const QString &query);

private:
    void updateTriggers();
    void onAdd(albert::QueryHandler*) override;
    void onRem(albert::QueryHandler*) override;

    std::map<QString,albert::QueryHandler*> trigger_map;
    std::vector<std::unique_ptr<Query>> queries;
};
