// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include "query.h"
#include "queryexecution.h"
#include "rankitem.h"
#include <QFuture>
#include <QString>
#include <chrono>
#include <vector>
namespace albert {
class GlobalQueryHandler;
class QueryResults;
}

class GlobalQueryResult : public albert::RankItem
{
public:
    explicit GlobalQueryResult(albert::GlobalQueryHandler *handler,
                               const albert::RankItem &rank_item) noexcept;
    ~GlobalQueryResult() noexcept;
    albert::GlobalQueryHandler *handler;
};


class GlobalQueryExecution final : public albert::QueryExecution,
                                   public albert::Query
{
    Q_OBJECT  // needed for invokable methods

public:

    GlobalQueryExecution(albert::Query &query,
                         std::vector<albert::GlobalQueryHandler*> query_handlers);
    ~GlobalQueryExecution();

private:

    // albert::Query
    bool isValid() const override;
    const albert::QueryHandler &handler() const override;
    QString string() const override;
    QString trigger() const override;

    // albert::QueryExecution
    void cancel() override;
    void fetchMore() override;
    bool canFetchMore() const override;
    bool isActive() const override;

    void addResultChunk();

    const std::vector<albert::GlobalQueryHandler*> handlers;
    QFuture<void> future;
    std::atomic_bool valid;
    bool active;
    std::vector<GlobalQueryResult> unordered_results;
    std::chrono::time_point<std::chrono::system_clock> start_timepoint;
    std::chrono::time_point<std::chrono::system_clock> finish_timepoint;
};
