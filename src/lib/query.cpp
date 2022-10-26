// Copyright (c) 2022 Manuel Schneider
#include "item.h"
#include "query.h"
#include "queryhandler.h"
#include "fallbackprovider.h"
#include "scopedtimeprinter.hpp"
#include <vector>
#include <QtConcurrent>
using namespace std;
using albert::SharedItem;
using albert::QueryHandler;
using albert::FallbackProvider;

const QString &albert::Query::trigger() const { return trigger_; }

const QString &albert::Query::string() const { return string_; }

bool albert::Query::isValid() const { return valid_; }

bool albert::Query::isFinished() const { return finished_; }

const vector<SharedItem> &albert::Query::results() const { return results_; }

const vector<SharedItem> &albert::Query::fallbacks() const { return fallbacks_; }

void albert::Query::add_(const SharedItem &item) { results_.push_back(item); }

void albert::Query::add_(SharedItem &&item) { results_.push_back(::move(item)); }

void albert::Query::set(vector<SharedItem> &&items) { results_ = ::move(items); emit resultsChanged(); }

void albert::Query::activateResult(uint item, uint action)
{
    results_[item]->actions()[action].function();
    // todo database
}

void albert::Query::activateFallback(uint item, uint action)
{
    fallbacks_[item]->actions()[action].function();
    // todo database
}

///////////////////////////////////////////////////////////////////////////////

Query::Query(std::set<albert::FallbackProvider*> fallback_handlers,
             albert::QueryHandler *query_handler,
             const QString &query_string,
             const QString &trigger_string)
{

    trigger_ = trigger_string;
    string_ = query_string;
    query_handler_ = query_handler;
    fallback_handlers_ = ::move(fallback_handlers);

    // Run query in background
    connect(&future_watcher, &decltype(future_watcher)::finished, this, &Query::finished);
    future_watcher.setFuture(QtConcurrent::run([this](){
        ScopedTimePrinter p("QUERY TOTAL %1 µs");

        auto raw_string = QString("%1%2").arg(trigger_, string_);
        for (auto fallback_handler : fallback_handlers_){
            auto fallbacks = fallback_handler->fallbacks(raw_string);
            fallbacks_.insert(fallbacks_.cend(), fallbacks.cbegin(), fallbacks.cend());
            // Todo sort by mru
        }

        query_handler_->handleQuery(*this);
    }));

}

void Query::cancel()
{
    valid_ = false;
}
