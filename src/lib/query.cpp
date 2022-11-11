// Copyright (c) 2022 Manuel Schneider

#include "albert/item.h"
#include "albert/logging.h"
#include "usagehistory.h"
#include "query.h"
#include "timeprinter.hpp"
#include <QtConcurrent>
#include <vector>
using namespace std;

Query::Query(albert::QueryHandler &query_handler, const QString &query_string, const QString &trigger_string)
    : query_handler(query_handler)
{
    synopsis_ = query_handler.synopsis();
    trigger_ = trigger_string;
    string_ = query_string;

    // Check regularly for new items and emit in _this_ thread
    connect(&timer_, &QTimer::timeout, [this, c=0]() mutable {
        if ((int)results_.size() != c){
            c = (int)results_.size();
            emit resultsChanged();
        }
    });
    timer_.start(25);
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
    timer_.stop();
    timer_.disconnect();
    valid_ = false;
}

void Query::run()
{
    // Run query in background
    connect(&future_watcher, &decltype(future_watcher)::finished, this, &Query::finished);
    future_watcher.setFuture(QtConcurrent::run([this](){
        TimePrinter tp(QString("TIME: %1 µs ['%2']").arg("%1", this->string()));
        try {
            this->query_handler.handleQuery(*this);
        } catch (const std::exception &e){
            WARN << "Handler thread threw" << e.what();
        }
    }));
}

void Query::clear()
{
    cancel();
    future_watcher.waitForFinished();  // rather busy wait here to avoid mutex against handlers
    results_.clear();
    emit resultsChanged();
}

const QString &Query::synopsis() const
{
    return synopsis_;
}

const QString &Query::trigger() const
{
    return trigger_;
}

const QString &Query::string() const
{
    return string_;
}

const std::vector<std::shared_ptr<albert::Item>> &Query::results() const
{
    return results_;
}

void Query::activateResult(uint i, uint a)
{
    auto *item = results_[i].get();
    auto action = item->actions()[a];
    action.function();
    UsageHistory::addActivation(string_, item->id(), action.id);
}

bool Query::isValid() const
{
    return valid_;
}

bool Query::isFinished() const
{
    return future_watcher.isFinished();
}

void Query::add(const std::shared_ptr<albert::Item> &item)
{
    results_.push_back(item);
}

void Query::add(std::shared_ptr<albert::Item> &&item)
{
    results_.push_back(::move(item));
}

void Query::add(std::vector<std::shared_ptr<albert::Item>> &&items)
{
    if (!results_.empty())
        results_ = ::move(items);
    else
        results_.insert(end(results_),
                        make_move_iterator(begin(items)),
                        make_move_iterator(end(items)));
}
