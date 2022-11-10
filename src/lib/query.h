// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/frontend.h"
#include "albert/queryhandler.h"
#include <QFutureWatcher>
#include <QString>
#include <QTimer>
#include <vector>
namespace albert {
class QueryHandler;
class Item;
}

class Query final:
        public albert::Query,
        public albert::QueryHandler::Query
{
public:
    Query(albert::QueryHandler &query_handler, const QString &query_string, const QString &trigger_string = QString());
    ~Query() final;

    void clear();
    void cancel() override;
    void run() override;

    const QString &synopsis() const override;
    const QString &trigger() const override;
    const QString &string() const override;
    const std::vector<std::shared_ptr<albert::Item>> &results() const override;
    bool isValid() const override;
    bool isFinished() const override;

    void add(const std::shared_ptr<albert::Item> &item) override;
    void add(std::shared_ptr<albert::Item> &&item) override;
    void add(std::vector<std::shared_ptr<albert::Item>> &&items) override;

    void activateResult(uint item, uint action) override;


private:
    QString synopsis_;
    QString trigger_;
    QString string_;
    std::vector<std::shared_ptr<albert::Item>> results_;
    bool valid_ = true;
    QTimer timer_;
    QFutureWatcher<void> future_watcher;
    const albert::QueryHandler &query_handler;
};

