// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "usagescoring.h"
#include <QObject>
#include <map>
#include <memory>
#include <mutex>
class QueryExecution;
class UsageScoring;
namespace albert {
class ExtensionRegistry;
class FallbackHandler;
class GlobalQueryHandler;
class TriggerQueryHandler;
}

class QueryEngine : public QObject
{
    Q_OBJECT

public:

    QueryEngine(albert::ExtensionRegistry&);
    
    std::unique_ptr<QueryExecution> query(const QString &query);

    UsageScoring usageScoring() const;  // thread-safe
    void setMemoryDecay(double);
    void setPrioritizePerfectMatch(bool);
    void storeItemActivation(const QString &query, const QString &extension,
                             const QString &item, const QString &action);

    std::map<QString, albert::TriggerQueryHandler*> triggerHandlers();
    std::map<QString, albert::GlobalQueryHandler*> globalHandlers();
    std::map<QString, albert::FallbackHandler*> fallbackHandlers();

    // Trigger handlers
    const std::map<QString, albert::TriggerQueryHandler*> &activeTriggerHandlers() const;
    QString trigger(const QString&) const;
    void setTrigger(const QString&, const QString&);
    bool fuzzy(const QString&) const;
    void setFuzzy(const QString&, bool);

    // Global handlers
    bool isEnabled(const QString&) const;
    void setEnabled(const QString&, bool = true);

    // Fallback handlers
    std::map<std::pair<QString, QString>, int> fallbackOrder() const;
    void setFallbackOrder(std::map<std::pair<QString, QString>, int>);

private:

    void updateActiveTriggers();
    void saveFallbackOrder() const;
    void loadFallbackOrder();

    albert::ExtensionRegistry &registry_;

    struct TriggerQueryHandler {
        TriggerQueryHandler(albert::TriggerQueryHandler *h, QString t, bool f):
            handler(h), trigger(t), fuzzy(f)
        {}
        albert::TriggerQueryHandler *handler;
        QString trigger;
        bool fuzzy;
    };

    struct GlobalQueryHandler {
        GlobalQueryHandler(albert::GlobalQueryHandler *h, bool e):
            handler(h), enabled(e)
        {}
        albert::GlobalQueryHandler *handler;
        bool enabled;
    };

    std::map<QString, TriggerQueryHandler> trigger_handlers_;
    std::map<QString, GlobalQueryHandler> global_handlers_;
    std::map<QString, albert::FallbackHandler*> fallback_handlers_;

    std::map<QString, albert::TriggerQueryHandler*> active_triggers_;
    std::map<std::pair<QString, QString>, int> fallback_order_;

    UsageScoring usage_scoring_;
    mutable std::mutex usage_scoring_mutex_;

signals:

    void handlerAdded();
    void handlerRemoved();

};
