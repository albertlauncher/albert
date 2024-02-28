// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/extensionwatcher.h"
#include <map>
#include <memory>
class QueryBase;

class QueryEngine : public QObject,
                    public albert::ExtensionWatcher<albert::TriggerQueryHandler>,
                    public albert::ExtensionWatcher<albert::GlobalQueryHandler>,
                    public albert::ExtensionWatcher<albert::FallbackHandler>
{
    Q_OBJECT

public:

    QueryEngine(albert::ExtensionRegistry&);
    
    std::unique_ptr<QueryBase> query(const QString &query);

    bool runEmptyQuery() const;
    void setRunEmptyQuery(bool);

    //
    // Trigger handlers
    //

    std::map<QString, albert::TriggerQueryHandler*> triggerHandlers();
    const std::map<QString, albert::TriggerQueryHandler*> &activeTriggerHandlers();

    bool isEnabled(albert::TriggerQueryHandler*) const;
    void setEnabled(albert::TriggerQueryHandler*, bool = true);

    void setTrigger(albert::TriggerQueryHandler*, const QString&);

    bool fuzzy(albert::TriggerQueryHandler*) const;
    void setFuzzy(albert::TriggerQueryHandler*, bool);

    //
    // Global handlers
    //

    std::map<QString, albert::GlobalQueryHandler*> globalHandlers();

    bool isEnabled(albert::GlobalQueryHandler*) const;
    void setEnabled(albert::GlobalQueryHandler*, bool = true);

    //
    // Fallback handlers
    //

    std::map<QString, albert::FallbackHandler*> fallbackHandlers();

    std::map<std::pair<QString, QString>, int> fallbackOrder() const;
    void setFallbackOrder(std::map<std::pair<QString, QString>, int>);

    bool isEnabled(albert::FallbackHandler*) const;
    void setEnabled(albert::FallbackHandler*, bool = true);

private:

    void updateActiveTriggers();

    void onAdd(albert::TriggerQueryHandler*) override;
    void onRem(albert::TriggerQueryHandler*) override;
    void onAdd(albert::GlobalQueryHandler*) override;
    void onRem(albert::GlobalQueryHandler*) override;
    void onRem(albert::FallbackHandler*) override;
    void onAdd(albert::FallbackHandler*) override;

    albert::ExtensionRegistry &registry_;
    bool runEmptyQuery_;
    std::map<QString, albert::TriggerQueryHandler*> enabled_trigger_handlers_;
    std::map<QString, albert::TriggerQueryHandler*> active_triggers_;
    std::map<QString, albert::GlobalQueryHandler*>  enabled_global_handlers_;
    std::map<QString, albert::FallbackHandler*> enabled_fallback_handlers_;

signals:

    void handlersChanged();

};
