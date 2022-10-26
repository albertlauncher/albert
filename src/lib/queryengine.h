// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "extensionwatcher.h"
#include "queryhandler.h"
#include "query.h"
#include "fallbackprovider.h"
#include <map>
#include <vector>
#include <memory>

class QueryEngine : public albert::ExtensionWatcher<albert::QueryHandler>,
                    public albert::ExtensionWatcher<albert::FallbackProvider>
{
public:
    /// NOTE You borrow! Watch destroyed signal! (Happens on unloading plugins)
    albert::Query* query(const QString &query);

private:
    void updateTriggers();
    void onReg(albert::QueryHandler*) override;
    void onDereg(albert::QueryHandler*) override;

    std::map<QString,albert::QueryHandler*> trigger_map;
    std::vector<std::unique_ptr<Query>> queries;
};
