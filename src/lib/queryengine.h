// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/indexqueryhandler.h"
#include "albert/extensions/queryhandler.h"
#include "albert/util/extensionwatcher.h"
#include "globalsearch.h"
#include "query.h"
#include <map>
#include <memory>
#include <set>

class QueryEngine : public albert::ExtensionWatcher<albert::QueryHandler>,
                    public albert::ExtensionWatcher<albert::IndexQueryHandler>

{
public:
    explicit QueryEngine(albert::ExtensionRegistry&);

    std::unique_ptr<albert::Query> query(const QString &query);

private:
    void updateTriggers();
    void onAdd(albert::QueryHandler*) override;
    void onRem(albert::QueryHandler*) override;
    void onAdd(albert::IndexQueryHandler*) override;

    std::map<QString,albert::QueryHandler*> trigger_map;
    GlobalSearch global_search_handler;
};
