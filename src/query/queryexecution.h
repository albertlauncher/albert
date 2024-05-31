// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "fallbackhandler.h"
#include "globalqueryhandler.h"
#include "itemsmodel.h"
#include "query.h"
#include "triggerqueryhandler.h"
#include <QFutureWatcher>
namespace albert { class Item; }
class QueryEngine;

class QueryExecution : public albert::Query
{
    Q_OBJECT  // needed for invokable methods

public:

    // explicit QueryBase(QueryEngine *e,
    //                    std::vector<albert::FallbackHandler*> fallback_handlers,
    //                    QString string);

    QueryExecution(QueryEngine *e,
                   std::vector<albert::FallbackHandler*> &&fallback_handlers,
                   albert::TriggerQueryHandler *query_handler,
                   QString string,
                   QString trigger);
    ~QueryExecution();

    void run();
    void cancel();

    QString trigger() const override final;
    QString string() const override final;
    QString synopsis() const override final;
    const bool &isValid() const override final;
    bool isFinished() const override final;
    bool isTriggered() const override final;

    QAbstractListModel *matches() override final;
    QAbstractListModel *fallbacks() override final;

    void activateMatch(uint item, uint action) override final;
    void activateFallback(uint item, uint action) override final;

    void add(const std::shared_ptr<albert::Item> &item) override;
    void add(std::shared_ptr<albert::Item> &&item) override;
    void add(const std::vector<std::shared_ptr<albert::Item>> &items) override;
    void add(std::vector<std::shared_ptr<albert::Item>> &&items) override;

protected:

    void runFallbackHandlers();
    void invokeCollectResults();
    Q_INVOKABLE void collectResults();

    QueryEngine *query_engine_;
    static uint query_count;
    const uint query_id;

    const QString trigger_;
    const QString string_;

    albert::TriggerQueryHandler * const query_handler_;
    const std::vector<albert::FallbackHandler*> fallback_handlers_;

    bool valid_ = true;

    QFutureWatcher<void> future_watcher_;

    // Mutable because global query handler needs adds items in handleTriggerQuery(…) _const_
    mutable std::vector<std::pair<albert::Extension*, std::shared_ptr<albert::Item>>> results_buffer_;
    std::mutex results_buffer_mutex_;

private:

    ItemsModel matches_;
    ItemsModel fallbacks_;

};


class GlobalQuery final : public QueryExecution,
                          public albert::TriggerQueryHandler
{
    Q_OBJECT  // needed for invokable methods

public:

    GlobalQuery(QueryEngine *e,
                std::vector<albert::FallbackHandler*> &&fallback_handlers,
                std::vector<albert::GlobalQueryHandler*> &&query_handlers,
                QString string);

    QString id() const override;
    QString name() const override;
    QString description() const override;
    void handleTriggerQuery(albert::Query *) override;

private:

    void addRankItems(std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator begin,
                      std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator end);

    std::vector<albert::GlobalQueryHandler*> query_handlers_;

};
