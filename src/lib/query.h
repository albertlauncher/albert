// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/frontend.h"
#include "albert/extensions/queryhandler.h"
#include "itemsmodel.h"
#include <QFutureWatcher>
#include <QString>
#include <QTimer>
#include <set>
#include <vector>
namespace albert {
class Item;
}

class Query final:
        public albert::Query,
        public albert::QueryHandler::Query
{
public:
    Query(std::set<albert::QueryHandler*> fallback_handlers,
          albert::QueryHandler &query_handler,
          QString query_string,
          QString trigger_string = QString());
    ~Query() final;

    // Frontend, Queryhandler
    const QString &trigger() const override;
    const QString &string() const override;

    // Frontend
    void cancel() override;
    void run() override;
    bool isFinished() const override;
    const QString &synopsis() const override;
    QAbstractListModel &matches() override;
    QAbstractListModel &fallbacks() override;
    QAbstractListModel *matchActions(uint item) const override;
    QAbstractListModel *fallbackActions(uint item) const override;
    void activateMatch(uint item, uint action) override;
    void activateFallback(uint item, uint action) override;

    // Queryhandler
    bool isValid() const override;
    void add(const std::shared_ptr<albert::Item> &item) override;
    void add(std::shared_ptr<albert::Item> &&item) override;
    void add(const std::vector<std::shared_ptr<albert::Item>> &items) override;
    void add(std::vector<std::shared_ptr<albert::Item>> &&items) override;

private:
    QString synopsis_;
    QString trigger_;
    QString string_;
    ItemsModel matches_;
    ItemsModel fallbacks_;
    bool valid_ = true;
    QFutureWatcher<void> future_watcher_;
    std::set<albert::QueryHandler*> fallback_handlers_{};
    const albert::QueryHandler &query_handler_;

    uint query_id;
    static uint query_count;
};

