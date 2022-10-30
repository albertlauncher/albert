// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/query.h"
#include <QFutureWatcher>
#include <QString>
#include <set>

namespace albert{
struct QueryHandler;
struct FallbackProvider;
}

class Query : public albert::Query
{
public:
    Query(std::set<albert::FallbackProvider*> fallback_handlers,
          albert::QueryHandler *query_handler,
          const QString &query_string,
          const QString &trigger_string = QString());
    ~Query();

    void cancel();
    void clear();

private:
    QFutureWatcher<void> future_watcher;
    albert::QueryHandler *query_handler_;
    std::set<albert::FallbackProvider *> fallback_handlers_;
};