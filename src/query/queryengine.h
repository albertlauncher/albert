// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "globalquery.h"
#include "usagescoring.h"
#include <QObject>
#include <map>
#include <memory>
namespace albert {
class ExtensionRegistry;
class FallbackHandler;
class GlobalQueryHandler;
class QueryHandler;
class UsageScoring;
class QueryResult;
namespace detail { class Query; }
}

class QueryEngine : public QObject
{
    Q_OBJECT

public:

    QueryEngine(albert::ExtensionRegistry&);

    std::unique_ptr<albert::detail::Query> query(QString query);

    albert::UsageScoring usageScoring() const;
    void setMemoryDecay(double);
    void setPrioritizePerfectMatch(bool);
    void storeItemActivation(const QString &query, const QString &extension,
                             const QString &item, const QString &action);

    std::map<QString, albert::QueryHandler*> triggerHandlers();
    std::map<QString, albert::GlobalQueryHandler*> globalHandlers();
    std::map<QString, albert::FallbackHandler*> fallbackHandlers();

    // Trigger handlers
    const std::map<QString, albert::QueryHandler*> &activeTriggerHandlers() const;
    QString trigger(const QString&) const;
    void setTrigger(const QString&, const QString&);
    bool fuzzy(const QString&) const;
    void setFuzzy(const QString&, bool);

    // Global handlers
    bool isEnabled(const QString&) const;
    void setEnabled(const QString&, bool = true);

    // Fallback handlers
    const std::map<std::pair<QString, QString>, int> &fallbackOrder() const;
    void setFallbackOrder(std::map<std::pair<QString, QString>, int>);

private:

    void updateActiveTriggers();
    void saveFallbackOrder() const;
    void loadFallbackOrder();
    std::vector<albert::QueryResult> fallbacks(const QString &query);

    albert::ExtensionRegistry &registry_;

    struct QueryHandler {
        albert::QueryHandler *handler;
        QString trigger;
        bool fuzzy;
    };
    std::map<QString, QueryHandler> trigger_handlers_;
    std::map<QString, albert::QueryHandler*> active_triggers_;

    GlobalQuery global_query_;
    std::map<QString, albert::GlobalQueryHandler*> global_handlers_;

    std::map<QString, albert::FallbackHandler*> fallback_handlers_;
    std::map<std::pair<QString, QString>, int> fallback_order_;

    albert::UsageScoring usage_scoring_;

signals:

    void handlerAdded();
    void handlerRemoved();
    void activeTriggersChanged();

};
