// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/frontend/query.h"
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "itemsmodel.h"
#include <QFutureWatcher>
namespace albert { class Item; }

class QueryBase : public albert::Query
{
public:
    explicit QueryBase(std::vector<albert::FallbackHandler*> fallback_handlers, QString string);

    void run() override final;
    void cancel() override final;
    bool isFinished() const override final;
    bool isTriggered() const override final;

    QAbstractListModel *matches() override final;
    QAbstractListModel *fallbacks() override final;
    QAbstractListModel *matchActions(uint item) const override final;
    QAbstractListModel *fallbackActions(uint item) const override final;

    void activateMatch(uint item, uint action) override final;
    void activateFallback(uint item, uint action) override final;

protected:
    virtual void run_() noexcept = 0;

    const uint query_id;
    const QString string_;
    bool valid_ = true;
    ItemsModel matches_;
    QFutureWatcher<void> future_watcher_;

private:
    void runFallbackHandlers();

    std::vector<albert::FallbackHandler*> fallback_handlers_;
    ItemsModel fallbacks_;
    static uint query_count;
};


class TriggerQuery final : public QueryBase,
                           public albert::TriggerQueryHandler::TriggerQuery
{
public:
    TriggerQuery(std::vector<albert::FallbackHandler*> &&fallback_handlers,
                          albert::TriggerQueryHandler *query_handler,
                          QString string, QString trigger);
    ~TriggerQuery() override;

    void run_() noexcept override;
    const bool &isValid() const override;

    QString trigger() const override;
    QString string() const override;
    QString synopsis() const override;

    void add(const std::shared_ptr<albert::Item> &item) override;
    void add(std::shared_ptr<albert::Item> &&item) override;
    void add(const std::vector<std::shared_ptr<albert::Item>> &items) override;
    void add(std::vector<std::shared_ptr<albert::Item>> &&items) override;

private:
    albert::TriggerQueryHandler *query_handler_;
    QString trigger_;
    QString synopsis_;
};


class GlobalQuery final : public QueryBase,
                          public albert::GlobalQueryHandler::GlobalQuery
{
public:
    GlobalQuery(std::vector<albert::FallbackHandler*> &&fallback_handlers,
                         std::vector<albert::GlobalQueryHandler*> &&query_handlers,
                         QString string);
    ~GlobalQuery() override;

    void run_() noexcept override;
    const bool &isValid() const override;

    QString trigger() const override { return {}; }
    QString string() const override;
    QString synopsis() const override { return {}; }

private:
    std::vector<albert::GlobalQueryHandler*> query_handlers_;
};
