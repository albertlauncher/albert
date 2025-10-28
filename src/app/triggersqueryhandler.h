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
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;
    void setFuzzyMatching(bool) override;
    bool supportsFuzzyMatching() const override;

private:

    std::shared_ptr<albert::Item> makeItem(const QString &trigger, Extension *handler) const;
    void updateTriggers();

    const QueryEngine &query_engine_;
    std::map<QString, QueryHandler *> handler_triggers_;
    std::shared_mutex handler_triggers_mutex_;
    std::atomic_bool fuzzy_;

};
