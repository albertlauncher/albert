// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "albert/util/timeprinter.hpp"
#include "query.h"
#include <QtConcurrent>
using namespace std;
using albert::Item;

uint Query::query_count = 0;

Query::Query(const std::set<albert::QueryHandler*>& fallback_handlers,
             albert::QueryHandler *query_handler,
             QString string,
             QString trigger):
        fallback_handlers_(fallback_handlers),
        query_handler_(query_handler),
        string_(std::move(string)),
        trigger_(std::move(trigger)),
        synopsis_(query_handler->synopsis()),
        query_id(query_count++)
{
    connect(&future_watcher_, &decltype(future_watcher_)::finished, this, &Query::finished);
}

Query::~Query()
{
    // Avoid segfaults when handler write on a deleted query
    if (!future_watcher_.isFinished()) {
        WARN << QString("Busy wait on query: #%1").arg(query_id);
        future_watcher_.waitForFinished();
    }
    DEBG << QString("Query deleted. [#%1 '%2']").arg(query_id).arg(string_);
}

const QString &Query::trigger() const { return trigger_; }

const QString &Query::string() const { return string_; }

const QString &Query::synopsis() const { return synopsis_; }

void Query::run()
{
    future_watcher_.setFuture(QtConcurrent::run([this](){
        albert::TimePrinter tp(QString("TIME: %1 µs ['%2':'%3']").arg("%1", query_handler_->id(), this->string()));
        try {
            for (auto fallback_handler : fallback_handlers_)
                fallbacks_.add(fallback_handler, fallback_handler->fallbacks(QString("%1%2").arg(trigger_, string_)));
            this->query_handler_->handleQuery(*this);
        } catch (const exception &e){
            WARN << "Handler thread threw" << e.what();
        }
    }));
}

void Query::cancel() { valid_ = false; }

bool Query::isValid() const { return valid_; }

bool Query::isFinished() const { return future_watcher_.isFinished(); }

QAbstractListModel &Query::matches() { return matches_; }

QAbstractListModel &Query::fallbacks() { return fallbacks_; }

static QAbstractListModel *buildActionsModel(Item &item)
{
    QStringList l;
    for (const auto &a : item.actions())
        l << a.text;
    return new QStringListModel(l);
}
 
QAbstractListModel *Query::matchActions(uint item) const { return buildActionsModel(*matches_.items[item].second); }

QAbstractListModel *Query::fallbackActions(uint item) const { return buildActionsModel(*fallbacks_.items[item].second); } 

static void activate(Query &query, ItemsModel &items, uint i, uint a)
{
    if (i<items.items.size()){
        auto &item = items.items[i];
        auto actions = item.second->actions();
        if (a<actions.size()){
            auto action = actions[a];
            action.function();
            emit query.activated(item.first->id(), item.second->id(), action.id);
        }
        else
            WARN << "Activated action index is invalid.";
    }
    else
        WARN << "Activated item index is invalid.";
}
 
void Query::activateMatch(uint i, uint a) { activate(*this, matches_, i, a); }

void Query::activateFallback(uint i, uint a) { activate(*this, fallbacks_, i, a); }

void Query::add(const shared_ptr<Item> &item) { matches_.add(query_handler_, item); }

void Query::add(shared_ptr<Item> &&item) { matches_.add(query_handler_, ::move(item)); }

void Query::add(const vector<shared_ptr<Item>> &items) { matches_.add(query_handler_, items); }

void Query::add(vector<shared_ptr<Item>> &&items) { matches_.add(query_handler_, ::move(items)); }


