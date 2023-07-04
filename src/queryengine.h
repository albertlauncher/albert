// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "albert/extensionwatcher.h"
#include "globalsearch.h"
#include <map>
#include <memory>
#include <set>
namespace albert{ class Query; }


class QueryEngine : public albert::ExtensionWatcher<albert::TriggerQueryHandler>,
                    public albert::ExtensionWatcher<albert::GlobalQueryHandler>,
                    public albert::ExtensionWatcher<albert::FallbackHandler>

{
public:

    explicit QueryEngine(albert::ExtensionRegistry&);

    std::shared_ptr<albert::Query> query(const QString &query);

    const std::map<albert::TriggerQueryHandler*, QString> &triggerHandlers();
    const std::set<albert::GlobalQueryHandler*> &globalHandlers();
    const std::set<albert::FallbackHandler*> &fallbackHandlers();

    bool isEnabled(albert::TriggerQueryHandler*) const;
    bool isEnabled(albert::GlobalQueryHandler*) const;
    bool isEnabled(albert::FallbackHandler*) const;

    bool setEnabled(albert::TriggerQueryHandler*, bool enabled = true);
    void setEnabled(albert::GlobalQueryHandler*, bool enabled = true);
    void setEnabled(albert::FallbackHandler*, bool enabled = true);

    const QString &trigger(albert::TriggerQueryHandler*) const;
    bool setTrigger(albert::TriggerQueryHandler*, const QString&);

    bool fuzzy() const;
    void setFuzzy(bool);

    double memoryDecay() const;
    void setMemoryDecay(double);

    bool prioritizePerfectMatch() const;
    void setPrioritizePerfectMatch(bool);

    const QString &separators() const;
    void setSeparators(const QString &);

private:
//    void updateActiveTriggers();
    void updateUsageScore() const;
    void onAdd(albert::TriggerQueryHandler*) override;
    void onRem(albert::TriggerQueryHandler *) override;
    void onAdd(albert::GlobalQueryHandler*) override;
    void onRem(albert::GlobalQueryHandler*) override;
    void onAdd(albert::FallbackHandler*) override;
    void onRem(albert::FallbackHandler*) override;

    std::map<albert::TriggerQueryHandler*, QString> trigger_handlers_;
    std::set<albert::GlobalQueryHandler*>  global_handlers_;
    std::set<albert::FallbackHandler*>     fallback_handlers_;

    std::map<albert::TriggerQueryHandler*, QString> enabled_trigger_handlers_;
    std::set<albert::GlobalQueryHandler*>  enabled_global_handlers_;
    std::set<albert::FallbackHandler*>     enabled_fallback_handlers_;

    GlobalSearch global_search_handler;
    bool fuzzy_;
    QString separators_;
    double memory_decay_;
    bool prioritize_perfect_match_;
};
