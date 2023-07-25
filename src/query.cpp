// Copyright (c) 2022 Manuel Schneider

#include "albert/extension/queryhandler/rankitem.h"
#include "albert/logging.h"
#include "albert/util/timeprinter.h"
#include "query.h"
#include "usagedatabase.h"
#include <QtConcurrent>
using namespace std;
using namespace albert;

uint QueryBase::query_count = 0;

QueryBase::QueryBase(vector<FallbackHandler*> fallback_handlers, QString string):
    fallback_handlers_(::move(fallback_handlers)),
    string_(::move(string)),
    matches_(this),  // Important for qml ownership determination
    fallbacks_(this),  // Important for qml ownership determination
    query_id(query_count++)
{
    connect(&future_watcher_, &decltype(future_watcher_)::finished, this, &QueryBase::finished);
}

QueryBase::~QueryBase()
{
    // Avoid segfaults when handler write on a deleted query
    if (!future_watcher_.isFinished()) {
        WARN << QString("Busy wait on query: #%1").arg(query_id);
        future_watcher_.waitForFinished();
    }
    DEBG << QString("Query deleted. [#%1 '%2']").arg(query_id).arg(string_);
}

void QueryBase::run()
{
    future_watcher_.setFuture(QtConcurrent::run([this](){
        try {
            runFallbackHandlers();
            run_();
        } catch (const exception &e){
            WARN << "Handler thread threw" << e.what();
        }
    }));
}

void QueryBase::cancel() { valid_ = false; }

bool QueryBase::isFinished() const { return future_watcher_.isFinished(); }

bool QueryBase::isTriggered() const { return !trigger().isEmpty(); }

QAbstractListModel *QueryBase::matches() { return &matches_; }

QAbstractListModel *QueryBase::fallbacks() { return &fallbacks_; }

QAbstractListModel *QueryBase::matchActions(uint i) const { return matches_.buildActionsModel(i); }

QAbstractListModel *QueryBase::fallbackActions(uint i) const { return fallbacks_.buildActionsModel(i); }

void QueryBase::activateMatch(uint i, uint a) { matches_.activate(this, i, a); }

void QueryBase::activateFallback(uint i, uint a) { fallbacks_.activate(this, i, a); }

void QueryBase::runFallbackHandlers()
{
    vector<pair<Extension*,RankItem>> fallbacks;
    for (auto *handler : fallback_handlers_)
        for (auto item : handler->fallbacks(QString("%1%2").arg(trigger(), string())))
            fallbacks.emplace_back(handler, RankItem(::move(item), 1));
    UsageHistory::applyScores(&fallbacks);
    sort(fallbacks.begin(), fallbacks.end(), [](const auto &a, const auto &b){ return a.second.score > b.second.score; });
    fallbacks_.add(fallbacks.begin(), fallbacks.end()); // TODO ranges
}

// ////////////////////////////////////////////////////////////////////////////

TriggerQuery::TriggerQuery(std::vector<FallbackHandler *> &&fallback_handlers,
                                             TriggerQueryHandler *query_handler,
                                             QString string, QString trigger):
    QueryBase(::move(fallback_handlers), ::move(string)),
    query_handler_(query_handler),
    trigger_(::move(trigger))
{
    synopsis_ = query_handler->synopsis();
}

QString TriggerQuery::trigger() const { return trigger_; }

QString TriggerQuery::string() const { return string_; }

QString TriggerQuery::synopsis() const { return synopsis_; }

const bool &TriggerQuery::isValid() const { return valid_; }

void TriggerQuery::add(const shared_ptr<Item> &item) { matches_.add(query_handler_, item); }

void TriggerQuery::add(shared_ptr<Item> &&item) { matches_.add(query_handler_, ::move(item)); }

void TriggerQuery::add(const vector<shared_ptr<Item>> &items) { matches_.add(query_handler_, items); }

void TriggerQuery::add(vector<shared_ptr<Item>> &&items) { matches_.add(query_handler_, ::move(items)); }

void TriggerQuery::run_()
{
    future_watcher_.setFuture(QtConcurrent::run([this](){
        TimePrinter tp(QString("TIME: %1 µs ['%2':'%3']").arg("%1", query_handler_->id(), string_));
        try {
            query_handler_->handleTriggerQuery(this);
        } catch (const exception &e){
            WARN << "Handler thread threw" << e.what();
        }
    }));
}

// ////////////////////////////////////////////////////////////////////////////

GlobalQuery::GlobalQuery(vector<FallbackHandler*> &&fallback_handlers,
                                           vector<GlobalQueryHandler*> &&query_handlers,
                                           QString string):
    QueryBase(::move(fallback_handlers), ::move(string)),
    query_handlers_(::move(query_handlers))
{
}

QString GlobalQuery::trigger() const { return {}; }

QString GlobalQuery::string() const { return string_; }

QString GlobalQuery::synopsis() const { return {}; }

const bool &GlobalQuery::isValid() const { return valid_; }

void GlobalQuery::run_()
{
    mutex rank_items_mutex;  // 6.4 Still no move semantics in QtConcurrent
    vector<pair<Extension*,RankItem>> rank_items;

    function<void(GlobalQueryHandler*)> map = [this, &rank_items_mutex, &rank_items](GlobalQueryHandler *handler) {
        try {
            TimePrinter tp(QString("TIME: %1 µs [%2:'%3']").arg("%1", handler->id(), string_));

            auto r = handler->handleGlobalQuery(this);
            if (r.empty())
                return;

            handler->applyUsageScore(&r);

            unique_lock lock(rank_items_mutex);
            rank_items.reserve(rank_items.size()+r.size());
            for (auto &rank_item : r)
                rank_items.emplace_back(handler, ::move(rank_item));

        } catch (const exception &e) {
            WARN << "Global search:" << handler->id() << "threw" << e.what();
        }
    };

    QtConcurrent::blockingMap(query_handlers_, map);


    TimePrinter tp(QString("TIME: %1 ms, Sorting global query '%2' results").arg("%1", string_));
    sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){
        if (a.second.score == b.second.score)
            return a.second.item->text() > b.second.item->text();
        else
            return a.second.score > b.second.score;
    });
    tp.restart(QString("TIME: %1 ms, adding global query '%2' results").arg("%1", string_));
    matches_.add(rank_items.begin(), rank_items.end()); // TODO ranges

//    //    auto it = rank_items.begin();
//    //    for (uint e = 0; pow(10,e)-1 < (uint)rank_items.size(); ++e){
//    //        auto begin = rank_items.begin()+(uint)pow(10u,e)-1;
//    //        auto end = rank_items.begin()+min((uint)pow(10u,e+1)-1, (uint)rank_items.size());
//    //        sort(begin, end, [](const auto &a, const auto &b){ return a.second.score > b.second.score; });
//    //         TODO c++20 ranges view
//    //    }
}
