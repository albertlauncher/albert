// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/frontend/query.h"
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/extensionwatcher.h"
#include <map>
#include <memory>
#include <set>

class QueryEngine:
    public albert::ExtensionWatcher<albert::TriggerQueryHandler>,
    public albert::ExtensionWatcher<albert::GlobalQueryHandler>,
    public albert::ExtensionWatcher<albert::FallbackHandler>
{
public:
    explicit QueryEngine(albert::ExtensionRegistry&);
    
    std::shared_ptr<albert::Query> query(const QString &query);

    std::map<QString, albert::TriggerQueryHandler*> triggerHandlers();
    std::map<QString, albert::GlobalQueryHandler*> globalHandlers();
    std::map<QString, albert::FallbackHandler*> fallbackHandlers();

    bool isActive(albert::TriggerQueryHandler*) const;
    bool isActive(albert::GlobalQueryHandler*) const;
    bool isActive(albert::FallbackHandler*) const;

    QString setActive(albert::TriggerQueryHandler*, bool = true);
    void setActive(albert::GlobalQueryHandler*, bool = true);
    void setActive(albert::FallbackHandler*, bool = true);

    bool isEnabled(albert::TriggerQueryHandler*) const;
    bool isEnabled(albert::GlobalQueryHandler*) const;
    bool isEnabled(albert::FallbackHandler*) const;

    QString setEnabled(albert::TriggerQueryHandler*, bool = true);
    void setEnabled(albert::GlobalQueryHandler*, bool = true);
    void setEnabled(albert::FallbackHandler*, bool = true);

    QString setTrigger(albert::TriggerQueryHandler*, const QString&);

    bool fuzzy(albert::TriggerQueryHandler*) const;
    void setFuzzy(albert::TriggerQueryHandler*, bool);

    bool runEmptyQuery() const;
    void setRunEmptyQuery(bool);

private:
    void onAdd(albert::TriggerQueryHandler*) override;
    void onAdd(albert::GlobalQueryHandler*) override;
    void onAdd(albert::FallbackHandler*) override;
    void onRem(albert::TriggerQueryHandler *) override;
    void onRem(albert::GlobalQueryHandler*) override;
    void onRem(albert::FallbackHandler*) override;

    std::map<QString, albert::TriggerQueryHandler*> enabled_trigger_handlers_;
    std::map<QString, albert::GlobalQueryHandler*>  enabled_global_handlers_;
    std::map<QString, albert::FallbackHandler*> enabled_fallback_handlers_;

    albert::ExtensionRegistry &registry_;
    std::map<QString, albert::TriggerQueryHandler*> active_triggers_;
    bool runEmptyQuery_;
};
