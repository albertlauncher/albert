// Copyright (c) 2022 Manuel Schneider

#include "albert/extension/queryhandler/rankitem.h"
#include "albert/logging.h"
#include "query.h"
#include "usagedatabase.h"
#include <QtConcurrent>
using namespace std;
using namespace albert;
using namespace chrono;

Q_LOGGING_CATEGORY(timeCat, "albert.query_runtimes")

uint QueryBase::query_count = 0;

QueryBase::QueryBase(vector<FallbackHandler*> fallback_handlers, QString string):
    query_id(query_count++),
    string_(::move(string)),
    matches_(this),  // Important for qml ownership determination
    fallback_handlers_(::move(fallback_handlers)),
    fallbacks_(this)  // Important for qml ownership determination
{
    connect(&future_watcher_, &decltype(future_watcher_)::finished, this, &QueryBase::finished);
}

void QueryBase::run()
{
    future_watcher_.setFuture(QtConcurrent::run([this](){
        try {
            auto tp = system_clock::now();
            runFallbackHandlers();
            run_();
            qCDebug(timeCat,).noquote()
                << QStringLiteral("\x1b[38;5;33m[%1 ms] #%2 '%3' TOTAL\x1b[0m")
                       .arg(duration_cast<milliseconds>(system_clock::now()-tp).count(), 6)
                       .arg(query_id).arg(string_);
        } catch (...){
            CRIT << "Unexpected exception in Query::run_!";
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

TriggerQuery::TriggerQuery(vector<FallbackHandler *> &&fallback_handlers,
                           TriggerQueryHandler *query_handler,
                           QString string, QString trigger):
    QueryBase(::move(fallback_handlers), ::move(string)),
    query_handler_(query_handler),
    trigger_(::move(trigger))
{
    synopsis_ = query_handler->synopsis();
}

TriggerQuery::~TriggerQuery()
{
    // Wait in derived class otherwise query is partially destroyed while handlers are still running
    cancel();
    if (!isFinished()) {
        WARN << QString("Busy wait on query: #%1").arg(query_id);
        future_watcher_.waitForFinished();
    }
    DEBG << QString("Query deleted. [#%1 '%2']").arg(query_id).arg(string());
}

QString TriggerQuery::trigger() const { return trigger_; }

QString TriggerQuery::string() const { return string_; }

QString TriggerQuery::synopsis() const { return synopsis_; }

const bool &TriggerQuery::isValid() const { return valid_; }

void TriggerQuery::run_() noexcept
{
    try {
        auto start = system_clock::now();
        query_handler_->handleTriggerQuery(this);
        qCDebug(timeCat,).noquote()
            << QStringLiteral("\x1b[38;5;244m[%1 ms] #%2 '%3' %4\x1b[0m")
                   .arg(duration_cast<milliseconds>(system_clock::now()-start).count(), 6)
                   .arg(query_id).arg(string_, query_handler_->id());
    }
    catch (const exception &e) {
        WARN << QString("TriggerQueryHandler '%1' threw exception:\n").arg(query_handler_->id()) << e.what();
    }
    catch (...) {
        WARN << QString("TriggerQueryHandler '%1' threw unknown exception:\n").arg(query_handler_->id());
    }
}

void TriggerQuery::add(const shared_ptr<Item> &item) { matches_.add(query_handler_, item); }

void TriggerQuery::add(shared_ptr<Item> &&item) { matches_.add(query_handler_, ::move(item)); }

void TriggerQuery::add(const vector<shared_ptr<Item>> &items) { matches_.add(query_handler_, items); }

void TriggerQuery::add(vector<shared_ptr<Item>> &&items) { matches_.add(query_handler_, ::move(items)); }

// ////////////////////////////////////////////////////////////////////////////

GlobalQuery::GlobalQuery(vector<FallbackHandler*> &&fallback_handlers,
                         vector<GlobalQueryHandler*> &&query_handlers,
                         QString string):
    QueryBase(::move(fallback_handlers), ::move(string)),
    query_handlers_(::move(query_handlers))
{
}

GlobalQuery::~GlobalQuery()
{
    // Wait in derived class otherwise query is partially destroyed while handlers are still running
    cancel();
    if (!isFinished()) {
        WARN << QString("Busy wait on query: #%1").arg(query_id);
        future_watcher_.waitForFinished();
    }
    DEBG << QString("Query deleted. [#%1 '%2']").arg(query_id).arg(string_);
}

QString GlobalQuery::string() const { return string_; }

const bool &GlobalQuery::isValid() const { return valid_; }

void GlobalQuery::run_() noexcept
{
    mutex rank_items_mutex;  // 6.4 Still no move semantics in QtConcurrent
    vector<pair<Extension*,RankItem>> rank_items;


    function<void(GlobalQueryHandler*)> map = [this, &rank_items_mutex, &rank_items](GlobalQueryHandler *handler)
    {
        // blocking map is not interruptible. end cancelled runs fast.
        if (!isValid())
            return;

        try {
            auto start = system_clock::now();

            auto r = handler->handleGlobalQuery(this);

            qCDebug(timeCat,).noquote()
                << QStringLiteral("\x1b[38;5;244m[%1 ms] #%2 '%3' %4\x1b[0m")
                       .arg(duration_cast<milliseconds>(system_clock::now()-start).count(), 6)
                       .arg(query_id).arg(string_, handler->id());

            if (r.empty())
                return;

            handler->applyUsageScore(&r);

            unique_lock lock(rank_items_mutex);
            rank_items.reserve(rank_items.size()+r.size());
            for (auto &rank_item : r)
                rank_items.emplace_back(handler, ::move(rank_item));

        }
        catch (const exception &e) {
            WARN << QString("GlobalQueryHandler '%1' threw exception:\n").arg(handler->id()) << e.what();
        }
        catch (...) {
            WARN << QString("GlobalQueryHandler '%1' threw unknown exception:\n").arg(handler->id());
        }
    };

    auto tp = system_clock::now();

    QtConcurrent::blockingMap(query_handlers_, map);

    qCDebug(timeCat,).noquote()
        << QStringLiteral("\x1b[38;5;244m[%1 ms] #%2 '%3' HANDLERS\x1b[0m")
               .arg(duration_cast<milliseconds>(system_clock::now()-tp).count(), 6)
               .arg(query_id).arg(string_);


    static const auto cmp = [](const auto &a, const auto &b){
        if (a.second.score == b.second.score)
            return a.second.item->text() > b.second.item->text();
        else
            return a.second.score > b.second.score;
    };

    tp = system_clock::now();

    if (rank_items.size() > 20)
    {
        auto mid = rank_items.begin() + 20;

        ranges::partial_sort(rank_items, mid, cmp);
        matches_.add(rank_items.begin(), mid);

        sort(mid, rank_items.end(), cmp);
        matches_.add(mid, rank_items.end());
    }
    else
    {
        ranges::sort(rank_items, cmp);
        matches_.add(rank_items.begin(), rank_items.end());
    }

    qCDebug(timeCat,).noquote()
        << QStringLiteral("\x1b[38;5;244m[%1 ms] #%2 '%3' SORT\x1b[0m")
               .arg(duration_cast<milliseconds>(system_clock::now()-tp).count(), 6)
               .arg(query_id).arg(string_);
}
