// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/query.h"
#include <QFutureWatcher>
#include <QString>
namespace albert { class QueryHandler; }

class Query final : public albert::Query
{
public:
    Query(albert::QueryHandler &query_handler, const QString &query_string, const QString &trigger_string = QString());
    ~Query() final;

    void cancel();
    void clear();

private:
    QFutureWatcher<void> future_watcher;
    const albert::QueryHandler &query_handler;
};