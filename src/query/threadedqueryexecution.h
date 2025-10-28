// Copyright (c) 2023-2025 Manuel Schneider

#include "queryexecution.h"
#include "threadedqueryhandler.h"
#include <QFuture>

class ThreadedQueryExecution final : public albert::QueryExecution,
                                     public albert::ThreadedQuery
{
    Q_OBJECT

public:

    ThreadedQueryExecution(albert::Query &query, albert::ThreadedQueryHandler &handler);
    ~ThreadedQueryExecution();

private:

    // QueryExecution
    void cancel() override;
    void fetchMore() override;
    bool canFetchMore() const override;
    bool isActive() const override;

    // Query
    bool isValid() const override;
    const albert::QueryHandler &handler() const override;
    QString string() const override;
    QString trigger() const override;

    // ThreadedQuery
    std::lock_guard<std::mutex> getLock() override;
    std::vector<std::shared_ptr<albert::Item>> &matches() override;
    void collect() override;

    Q_INVOKABLE void collectInMainThread();

    albert::ThreadedQueryHandler &handler_;
    QFuture<void> future;
    std::mutex match_buffer_mutex;
    std::vector<std::shared_ptr<albert::Item>> match_buffer;
    std::atomic_bool valid;
    bool active;

};
