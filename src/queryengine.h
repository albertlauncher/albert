// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/queryhandler.h"
#include "albert/util/extensionwatcher.h"
#include "globalsearch.h"
#include <map>
#include <memory>
#include <set>
namespace albert{ class Query; }


class QueryEngine : public albert::ExtensionWatcher<albert::TriggerQueryHandler>,
                    public albert::ExtensionWatcher<albert::GlobalQueryHandler>,
                    public albert::ExtensionWatcher<albert::IndexQueryHandler>,
                    public albert::ExtensionWatcher<albert::FallbackHandler>

{
public:
    struct HandlerConfig {
        QString trigger;
        bool enabled;
    };

    explicit QueryEngine(albert::ExtensionRegistry&);

    std::shared_ptr<albert::Query> query(const QString &query);

    const std::map<albert::TriggerQueryHandler*,HandlerConfig> &handlerConfig() const;
    void setTrigger(albert::TriggerQueryHandler*, const QString&);
    void setEnabled(albert::TriggerQueryHandler*, bool);
    const std::map<QString,albert::TriggerQueryHandler*> &activeTriggers() const;

    bool fuzzy() const;
    void setFuzzy(bool);

    double memoryDecay() const;
    void setMemoryDecay(double);

    bool prioritizePerfectMatch() const;
    void setPrioritizePerfectMatch(bool);

    const QString &separators() const;
    void setSeparators(const QString &);

private:
    void updateActiveTriggers();
    void updateUsageScore() const;
    void onAdd(albert::TriggerQueryHandler*) override;
    void onRem(albert::TriggerQueryHandler *) override;
    void onAdd(albert::GlobalQueryHandler*) override;
    void onRem(albert::GlobalQueryHandler*) override;
    void onAdd(albert::IndexQueryHandler*) override;
    void onRem(albert::IndexQueryHandler*) override;
    void onAdd(albert::FallbackHandler*) override;
    void onRem(albert::FallbackHandler*) override;

    std::set<albert::FallbackHandler*> fallback_handlers_;
    std::set<albert::TriggerQueryHandler*> trigger_query_handlers_;
    std::set<albert::IndexQueryHandler*> index_query_handlers_;

    GlobalSearch global_search_handler;
    std::map<albert::TriggerQueryHandler*,HandlerConfig> query_handler_configs_;
    std::map<QString,albert::TriggerQueryHandler*> active_triggers_;
    bool fuzzy_;
    QString separators_;
    double memory_decay_;
    bool prioritize_perfect_match_;
};
