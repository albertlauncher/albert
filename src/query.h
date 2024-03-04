// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/frontend/query.h"
#include "albert/query/fallbackprovider.h"
#include "albert/query/globalqueryhandler.h"
#include "albert/query/triggerqueryhandler.h"
#include "itemsmodel.h"
#include <QFutureWatcher>
namespace albert { class Item; }

class QueryBase : public albert::Query
{
public:

    explicit QueryBase(QueryEngine *e,
                       std::vector<albert::FallbackHandler*> fallback_handlers,
                       QString string);

    void run();
    void cancel();

    bool isFinished() const override final;
    bool isTriggered() const override final;

    QAbstractListModel *matches() override final;
    QAbstractListModel *fallbacks() override final;

    void activateMatch(uint item, uint action) override final;
    void activateFallback(uint item, uint action) override final;

protected:

    void runFallbackHandlers();
    virtual void run_() noexcept = 0;

    QueryEngine *query_engine_;
    static uint query_count;
    const uint query_id;
    bool valid_ = true;
    const QString string_;
    std::vector<albert::FallbackHandler*> fallback_handlers_;
    ItemsModel fallbacks_;
    ItemsModel matches_;
    QFutureWatcher<void> future_watcher_;

};


class TriggerQuery final : public QueryBase, public albert::TriggerQueryHandler::TriggerQuery
{
    Q_OBJECT

public:

    TriggerQuery(QueryEngine *e,
                 std::vector<albert::FallbackHandler*> &&fallback_handlers,
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

    Q_INVOKABLE void collectResults();

    std::vector<std::shared_ptr<albert::Item>> results_buffer_;
    std::mutex results_buffer_mutex_;
    albert::TriggerQueryHandler *query_handler_;
    QString trigger_;
    QString synopsis_;

};


class GlobalQuery final : public QueryBase, public albert::GlobalQueryHandler::GlobalQuery
{
public:

    GlobalQuery(QueryEngine *e,
                std::vector<albert::FallbackHandler*> &&fallback_handlers,
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
