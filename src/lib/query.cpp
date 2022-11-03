// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "albert/queryhandler.h"
#include "query.h"
#include "timeprinter.hpp"
#include <QtConcurrent>

Query::Query(albert::QueryHandler &query_handler, const QString &query_string, const QString &trigger_string)
    : query_handler(query_handler)
{
    trigger_ = trigger_string;
    string_ = query_string;

    // Run query in background
    connect(&future_watcher, &decltype(future_watcher)::finished, this, &Query::finished);
    future_watcher.setFuture(QtConcurrent::run([this](){
        TimePrinter tp(QString("TIME: %1 µs ['%2']").arg("%1", this->string()));
        this->query_handler.handleQuery(*this);
    }));

}

Query::~Query()
{
    // Avoid segfaults when handler write on a deleted query
    if (!future_watcher.isFinished()) {
        WARN << QString("Busy wait on query. Does '%1' handle cancellation well?").arg(query_handler.id());
        future_watcher.waitForFinished();
    }
}

void Query::cancel()
{
    valid_ = false;
}

void Query::clear()
{
    cancel();
    future_watcher.waitForFinished();  // rather busy wait here to avoid mutex against handlers
    results_.clear();
    emit resultsChanged();
}
