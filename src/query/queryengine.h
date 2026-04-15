// Copyright (c) 2023-2026 Manuel Schneider

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

    double memoryDecay() const;
    void setMemoryDecay(double);

    bool prioritizePerfectMatch() const;
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
    void updateUsageScoring();

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

    /// The exponential decay applied to usage scores based on recency.
    /// This value adjusts the influence of recent item activations using a geometric weighting
    /// scheme: each activation contributes a weight of 1 / (memory_decay^recency).
    /// A value of 1.0 disables decay, assigning equal weight to all activations and the score of
    /// an item is the sum of its activations (Most Frequently Used).
    /// A value of 0.5 implies that for any activation a_i in history the sum of all older
    /// activations can not exceed the weight of a_i (Most Recently Used).
    /// Valid range: [0.5, 1.0]
    double memory_decay_;

    /// If `true` perfect matches should be prioritized even if their usage score is lower.
    bool prioritize_perfect_match_;

    albert::UsageScoring usage_scoring_;

signals:

    void queryHandlerAdded(albert::QueryHandler*);
    void queryHandlerRemoved(albert::QueryHandler*);

    void globalQueryHandlerAdded(albert::GlobalQueryHandler*);
    void globalQueryHandlerRemoved(albert::GlobalQueryHandler*);

    void fallbackHandlerAdded(albert::FallbackHandler*);
    void fallbackHandlerRemoved(albert::FallbackHandler*);

    void activeTriggersChanged();

};
