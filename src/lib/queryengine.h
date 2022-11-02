// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensionwatcher.h"
#include "albert/queryhandler.h"
#include "globalsearch.h"
#include "query.h"
#include <map>
#include <memory>
#include <vector>

class QueryEngine : public albert::ExtensionWatcher<albert::QueryHandler>
{
public:
    QueryEngine(albert::ExtensionRegistry&);

    std::unique_ptr<albert::Query> query(const QString &query);

private:
    void updateTriggers();
    void onAdd(albert::QueryHandler*) override;
    void onRem(albert::QueryHandler*) override;

    std::map<QString,albert::QueryHandler*> trigger_map;
    std::set<Query*> alive_queries;
    GlobalSearch global_search_handler;
};
