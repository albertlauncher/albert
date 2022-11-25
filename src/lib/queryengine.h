// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/indexqueryhandler.h"
#include "albert/util/extensionwatcher.h"
#include "globalsearch.h"
#include <map>
#include <memory>
#include <set>
namespace albert{ class Query; }


class QueryEngine:
        public albert::ExtensionWatcher<albert::QueryHandler>,
        public albert::ExtensionWatcher<albert::GlobalQueryHandler>,
        public albert::ExtensionWatcher<albert::IndexQueryHandler>

{
public:
    struct HandlerConfig {
        QString trigger;
        bool enabled;
    };

    explicit QueryEngine(albert::ExtensionRegistry&);

    std::unique_ptr<albert::Query> query(const QString &query);

    const std::map<albert::QueryHandler*,HandlerConfig> &handlerConfig() const;
    void setTrigger(albert::QueryHandler*, const QString&);
    void setEnabled(albert::QueryHandler*, bool);
    const std::map<QString,albert::QueryHandler*> &activeTriggers() const;

    bool fuzzy() const;
    void setFuzzy(bool);

    double memoryDecay() const;
    void setMemoryDecay(double);

    double memoryWeight() const;
    void setMemoryWeight(double);

    const QString &separators() const;
    void setSeparators(const QString &);

private:
    void updateActiveTriggers();
    void updateUsageScore() const;
    void onAdd(albert::QueryHandler*) override;
    void onRem(albert::QueryHandler*) override;
    void onAdd(albert::GlobalQueryHandler*) override;
    void onRem(albert::GlobalQueryHandler*) override;
    void onAdd(albert::IndexQueryHandler*) override;
    void onRem(albert::IndexQueryHandler*) override;

    std::set<albert::QueryHandler*> query_handlers_;
    std::set<albert::IndexQueryHandler*> index_query_handlers_;

    GlobalSearch global_search_handler;
    std::map<albert::QueryHandler*,HandlerConfig> query_handler_configs_;
    std::map<QString,albert::QueryHandler*> active_triggers_;
    bool fuzzy_;
    QString separators_;
    double memory_decay_;
    double memory_weight_;
};
