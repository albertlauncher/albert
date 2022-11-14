// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "query.h"
#include "timeprinter.hpp"
#include "usagehistory.h"
#include <QtConcurrent>
#include <utility>
#include <vector>
using namespace std;
using albert::Item;

Query::Query(set<albert::QueryHandler*> fallback_handlers,
             albert::QueryHandler &query_handler,
             QString query_string,
             QString trigger_string):
        fallback_handlers_(::move(fallback_handlers)),
        query_handler_(query_handler),
        synopsis_(query_handler.synopsis()),
        trigger_(::move(trigger_string)),
        string_(::move(query_string))
{
    connect(&future_watcher_, &decltype(future_watcher_)::finished, this, &Query::finished);
}

Query::~Query()
{
    // Avoid segfaults when handler write on a deleted query
    if (!future_watcher_.isFinished()) {
        WARN << QString("Busy wait on query. Does '%1' handle cancellation well?").arg(query_handler_.id());
        future_watcher_.waitForFinished();
    }
}

void Query::run()
{
    future_watcher_.setFuture(QtConcurrent::run([this](){
        TimePrinter tp(QString("TIME: %1 µs ['%2']").arg("%1", this->string()));
        try {
            for (auto fallback_handler : fallback_handlers_)
                fallbacks_.add(fallback_handler->fallbacks(QString("%1%2").arg(trigger_, string_)));
            this->query_handler_.handleQuery(*this);
        } catch (const exception &e){
            WARN << "Handler thread threw" << e.what();
        }
    }));
}

void Query::cancel()
{
    valid_ = false;
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

bool Query::isValid() const
{
    return valid_;
}

void Query::add(const shared_ptr<Item> &item)
{
    matches_.add(item);
}

void Query::add(shared_ptr<Item> &&item)
{
    matches_.add(::move(item));
}

void Query::add(const vector<shared_ptr<Item>> &items)
{
    matches_.add(items);
}

void Query::add(vector<shared_ptr<Item>> &&items)
{
    matches_.add(::move(items));
}

QAbstractListModel &Query::matches()
{
    return matches_;
}

QAbstractListModel &Query::fallbacks()
{
    return fallbacks_;
}

static QAbstractListModel *buildActionsModel(Item &item)
{
    QStringList l;
    for (const auto &a : item.actions())
        l << a.text;
    return new QStringListModel(l);
}

QAbstractListModel *Query::matchActions(uint item) const
{
    return buildActionsModel(*matches_.items[item].get());
}

QAbstractListModel *Query::fallbackActions(uint item) const
{
    return buildActionsModel(*fallbacks_.items[item].get());
}

void Query::activateMatch(uint i, uint a)
{
    auto *item = matches_.items[i].get();
    auto action = item->actions()[a];
    action.function();
    UsageHistory::addActivation(string_, item->id(), action.id);
}

void Query::activateFallback(uint i, uint a)
{
    auto *item = fallbacks_.items[i].get();
    auto action = item->actions()[a];
    action.function();
    UsageHistory::addActivation(string_, item->id(), action.id);
}

