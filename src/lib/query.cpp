// Copyright (c) 2022 Manuel Schneider
#include "item.h"
#include "query.h"
#include "queryhandler.h"
#include "scopedtimeprinter.hpp"
#include <vector>
#include <QtConcurrent>
using namespace std;
using albert::SharedItem;
using albert::QueryHandler;

const QString &albert::Query::trigger() const { return trigger_; }

const QString &albert::Query::string() const { return string_; }

bool albert::Query::isValid() const { return valid_; }

bool albert::Query::isFinished() const { return finished_; }

const vector<SharedItem> &albert::Query::results() const { return results_; }

void albert::Query::add_(const SharedItem &item) { results_.push_back(item); }

void albert::Query::add_(SharedItem &&item) { results_.push_back(::move(item)); }

void albert::Query::set(vector<SharedItem> &&items) { results_ = ::move(items); emit resultsChanged(); }

void albert::Query::activateResult(uint item, uint action)
{
    results_[item]->actions()[action].function();
    // todo database
}

///////////////////////////////////////////////////////////////////////////////

Query::Query(albert::QueryHandler &query_handler, const QString &query_string, const QString &trigger_string)
    : query_handler(query_handler)
{
    trigger_ = trigger_string;
    string_ = query_string;

    // Run query in background
    connect(&future_watcher, &decltype(future_watcher)::finished, this, &Query::finished);
    future_watcher.setFuture(QtConcurrent::run([this](){
        ScopedTimePrinter p("QUERY TOTAL %1 µs");
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
