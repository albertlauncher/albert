// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "globalqueryhandler.h"
#include <QCoreApplication>
#include <shared_mutex>
class QueryEngine;

class TriggersQueryHandler : public QObject, public albert::GlobalQueryHandler
{
    Q_OBJECT

public:

    TriggersQueryHandler(const QueryEngine &query_engine);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    std::vector<albert::RankItem> rankItems(albert::QueryContext &) override;
    bool supportsFuzzyMatching() const override;
    void onFuzzyMatchingChanged(bool) override;

private:

    struct TriggerHandler {
        QString id;
        QString name;
        QString description;
        QString trigger;
    };

    std::shared_ptr<albert::Item> makeItem(const TriggerHandler &) const;
    void updateTriggers();

    const QueryEngine &query_engine_;
    std::vector<TriggerHandler> trigger_handlers_;
    std::shared_mutex trigger_handlers_mutex_;
    std::atomic_bool fuzzy_;

};
