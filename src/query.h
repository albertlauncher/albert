// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/frontend/query.h"
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "itemsmodel.h"
#include <QFutureWatcher>
#include <set>
namespace albert { class Item; }

class QueryBase : public albert::Query
{
public:
    explicit QueryBase(std::vector<albert::FallbackHandler*> fallback_handlers, QString string);

    void run() override;
    void cancel() override;

    bool isFinished() const override;
    bool isTriggered() const override;

    QAbstractListModel *matches() override;
    QAbstractListModel *fallbacks() override;
    QAbstractListModel *matchActions(uint item) const override;
    QAbstractListModel *fallbackActions(uint item) const override;
    void activateMatch(uint item, uint action) override;
    void activateFallback(uint item, uint action) override;

protected:
    void runFallbackHandlers();
    virtual void run_() = 0;

    std::vector<albert::FallbackHandler*> fallback_handlers_;
    QString string_;
    ItemsModel matches_;
    ItemsModel fallbacks_;
    bool valid_ = true;
    QFutureWatcher<void> future_watcher_;

    uint query_id;
    static uint query_count;
};


class TriggerQuery : public QueryBase, public albert::TriggerQueryHandler::TriggerQuery
{
    albert::TriggerQueryHandler *query_handler_;
    QString trigger_;
    QString synopsis_;
public:
    TriggerQuery(std::vector<albert::FallbackHandler*> &&fallback_handlers,
                          albert::TriggerQueryHandler *query_handler,
                          QString string, QString trigger);
    ~TriggerQuery() override;

    void run_() override;

    QString trigger() const override;
    QString string() const override;
    QString synopsis() const override;
    const bool &isValid() const override;
    void add(const std::shared_ptr<albert::Item> &item) override;
    void add(std::shared_ptr<albert::Item> &&item) override;
    void add(const std::vector<std::shared_ptr<albert::Item>> &items) override;
    void add(std::vector<std::shared_ptr<albert::Item>> &&items) override;
};


class GlobalQuery : public QueryBase, public albert::GlobalQueryHandler::GlobalQuery
{
    std::vector<albert::GlobalQueryHandler*> query_handlers_;
public:
    GlobalQuery(std::vector<albert::FallbackHandler*> &&fallback_handlers,
                         std::vector<albert::GlobalQueryHandler*> &&query_handlers,
                         QString string);
    ~GlobalQuery() override;

    void run_() override;

    QString trigger() const override;
    QString string() const override;
    QString synopsis() const override;
    const bool &isValid() const override;
};
