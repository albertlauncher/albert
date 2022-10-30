// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "extensionwatcher.h"
#include "query.h"
#include "queryhandler.h"
#include <map>
#include <memory>
#include <vector>

class QueryEngine : public albert::ExtensionWatcher<albert::QueryHandler>
{
public:
    std::unique_ptr<albert::Query> query(const QString &query);

private:
    void updateTriggers();
    void onAdd(albert::QueryHandler*) override;
    void onRem(albert::QueryHandler*) override;

    std::map<QString,albert::QueryHandler*> trigger_map;
    std::set<Query*> alive_queries;
};
