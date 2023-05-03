// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/frontend.h"
#include "albert/extensions/queryhandler.h"
#include "itemsmodel.h"
#include <QFutureWatcher>
#include <set>
namespace albert { class Item; }

class Query: public albert::Query,
             public albert::TriggerQueryHandler::TriggerQuery,
             public albert::GlobalQueryHandler::GlobalQuery
{
    Q_OBJECT
public:
    explicit Query(const std::set<albert::FallbackHandler*> &fallback_handlers,
                   albert::TriggerQueryHandler *query_handler,
                   QString string,
                   QString trigger = {});
    ~Query() override;

    // Interfaces
    const QString &trigger() const override;
    const QString &string() const override;
    const QString &synopsis() const override;

    void run() override;
    void cancel() override;
    bool isValid() const override;
    bool isFinished() const override;

    QAbstractListModel &matches() override;
    QAbstractListModel &fallbacks() override;
    QAbstractListModel *matchActions(uint item) const override;
    QAbstractListModel *fallbackActions(uint item) const override;
    void activateMatch(uint item, uint action) override;
    void activateFallback(uint item, uint action) override;

    void add(const std::shared_ptr<albert::Item> &item) override;
    void add(std::shared_ptr<albert::Item> &&item) override;
    void add(const std::vector<std::shared_ptr<albert::Item>> &items) override;
    void add(std::vector<std::shared_ptr<albert::Item>> &&items) override;

    const std::set<albert::FallbackHandler*> &fallback_handlers_;
    albert::TriggerQueryHandler *query_handler_;
    QString string_;
    QString trigger_;
    QString synopsis_;
    ItemsModel matches_;
    ItemsModel fallbacks_;
    bool valid_ = true;
    QFutureWatcher<void> future_watcher_;

    uint query_id;
    static uint query_count;

};
