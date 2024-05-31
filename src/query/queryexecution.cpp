// Copyright (c) 2022-2024 Manuel Schneider

#include "logging.h"
#include "queryengine.h"
#include "queryexecution.h"
#include "usagedatabase.h"
#include <QtConcurrent>
using namespace albert;
using namespace std::chrono;
using namespace std;

Q_LOGGING_CATEGORY(timeCat, "albert.query_runtimes")

uint QueryExecution::query_count = 0;

QueryExecution::QueryExecution(QueryEngine *e,
                               vector<FallbackHandler *> &&fallback_handlers,
                               TriggerQueryHandler *query_handler,
                               QString string,
                               QString trigger):
    query_engine_(e),
    query_id(query_count++),
    trigger_(::move(trigger)),
    string_(::move(string)),
    query_handler_(query_handler),
    fallback_handlers_(::move(fallback_handlers)),
    valid_(true),
    matches_(this),  // Important for qml ownership determination
    fallbacks_(this)  // Important for qml ownership determination
{
    connect(&future_watcher_, &decltype(future_watcher_)::finished, this, &Query::finished);
}

QueryExecution::~QueryExecution()
{
    // Wait in derived class otherwise query is partially destroyed while handlers are still running
    cancel();
    if (!isFinished()) {
        WARN << QString("Busy wait on query: #%1").arg(query_id);
        // there may be some queued collectResults calls
        QCoreApplication::processEvents();
        future_watcher_.waitForFinished();
    }
    DEBG << QString("Query deleted. [#%1 '%2']").arg(query_id).arg(string());
}

void QueryExecution::run()
{
    future_watcher_.setFuture(QtConcurrent::run([this](){
        try {
            runFallbackHandlers();
            auto tp = system_clock::now();
            query_handler_->handleTriggerQuery(this);
            qCDebug(timeCat,).noquote()
                << QStringLiteral("\x1b[38;5;33m│%1 ms│ TRIGGER |%2│ #%3  '%4' '%5' \x1b[0m")
                       .arg(duration_cast<milliseconds>(system_clock::now() - tp).count(), 6)
                       .arg(matches_.rowCount(), 6)
                       .arg(query_id)
                       .arg(trigger_, string_);
        }
        catch (const exception &e) {
            WARN << QString("TriggerQueryHandler '%1' threw exception:\n").arg(query_handler_->id()) << e.what();
        }
        catch (...){
            CRIT << "Unexpected exception in QueryExecution::run()!";
        }
    }));
}

void QueryExecution::cancel() { valid_ = false; }

QString QueryExecution::trigger() const { return trigger_; }

QString QueryExecution::string() const { return string_; }

QString QueryExecution::synopsis() const { return query_handler_->synopsis(); }

const bool &QueryExecution::isValid() const { return valid_; }

bool QueryExecution::isFinished() const { return future_watcher_.isFinished(); }

bool QueryExecution::isTriggered() const { return !trigger().isEmpty(); }

QAbstractListModel *QueryExecution::matches() { return &matches_; }

QAbstractListModel *QueryExecution::fallbacks()  { return &fallbacks_; }

void QueryExecution::activateMatch(uint i, uint a) { matches_.activate(this, i, a); }

void QueryExecution::activateFallback(uint i, uint a) { fallbacks_.activate(this, i, a); }

void QueryExecution::add(const shared_ptr<Item> &item)
{
    unique_lock lock(results_buffer_mutex_);

    results_buffer_.emplace_back(query_handler_, item);

    if (valid_)
        invokeCollectResults();
}

void QueryExecution::add(shared_ptr<Item> &&item)
{
    unique_lock lock(results_buffer_mutex_);

    results_buffer_.emplace_back(query_handler_, ::move(item));

    if (valid_)
        invokeCollectResults();
}

void QueryExecution::add(const vector<shared_ptr<Item>> &items)
{
    unique_lock lock(results_buffer_mutex_);

    for (const auto &item : items)
        results_buffer_.emplace_back(query_handler_, item);

    if (valid_)
        invokeCollectResults();
}

void QueryExecution::add(vector<shared_ptr<Item>> &&items)
{
    unique_lock lock(results_buffer_mutex_);

    for (auto &item : items)
        results_buffer_.emplace_back(query_handler_, ::move(item));

    if (valid_)
        invokeCollectResults();
}

void QueryExecution::invokeCollectResults()
{
    QMetaObject::invokeMethod(this, &QueryExecution::collectResults, Qt::QueuedConnection);
}

void QueryExecution::runFallbackHandlers()
{
    const auto &o = query_engine_->fallbackOrder();

    vector<pair<Extension*,RankItem>> fallbacks;
    for (auto *handler : fallback_handlers_)
        for (auto item : handler->fallbacks(QString("%1%2").arg(trigger(), string())))
            if (auto it = o.find(make_pair(handler->id(), item->id())); it == o.end())
                fallbacks.emplace_back(handler, RankItem(::move(item), 0));
            else
                fallbacks.emplace_back(handler, RankItem(::move(item), it->second));

    sort(fallbacks.begin(), fallbacks.end(),
         [](const auto &a, const auto &b){ return a.second.score > b.second.score; });

    fallbacks_.add(fallbacks.begin(), fallbacks.end()); // TODO ranges
}

void QueryExecution::collectResults()
{
    // Rationale:
    // Queued signals from other threads may fire multple times which
    // messes up the frontend state machines. So we collect the results in
    // the main thread using a buffer.
    unique_lock lock(results_buffer_mutex_);
    if (!results_buffer_.empty())
    {
        matches_.add(results_buffer_.begin(), results_buffer_.end());
        results_buffer_.clear();
    }
}

// ////////////////////////////////////////////////////////////////////////////

GlobalQuery::GlobalQuery(QueryEngine *e,
                         vector<FallbackHandler*> &&fallback_handlers,
                         vector<GlobalQueryHandler*> &&query_handlers,
                         QString string):
    QueryExecution(e, ::move(fallback_handlers), this, ::move(string), {}),
    query_handlers_(::move(query_handlers))
{
}

// GlobalQuery::~GlobalQuery()
// {
//     // Wait in derived class otherwise query is partially destroyed while handlers are still running
//     cancel();
//     if (!isFinished()) {
//         WARN << QString("Busy wait on query: #%1").arg(query_id);
//         future_watcher_.waitForFinished();
//     }
//     DEBG << QString("Query deleted. [#%1 '%2']").arg(query_id).arg(string_);
// }


QString GlobalQuery::id() const
{ return QStringLiteral("globalquery"); }

QString GlobalQuery::name() const
{ return QStringLiteral("Global query"); }

QString GlobalQuery::description() const
{ return QStringLiteral("Runs a bunch of global query handlers"); }

void GlobalQuery::handleTriggerQuery(albert::Query *)
{
    mutex rank_items_mutex;  // 6.4 Still no move semantics in QtConcurrent
    vector<pair<Extension*,RankItem>> rank_items;


    qCDebug(timeCat,).noquote() << QStringLiteral("\x1b[38;5;244m│ Handling│  Scoring│ Count│\x1b[0m");

    function<void(GlobalQueryHandler*)> map = [this, &rank_items_mutex, &rank_items](GlobalQueryHandler *handler)
    {
        // blocking map is not interruptible. end cancelled runs fast.
        if (!isValid())
            return;

        try {
            auto t = system_clock::now();

            vector<RankItem> results;
            if (string_.isEmpty())
                for (auto &item : handler->handleEmptyQuery(this))
                    results.emplace_back(::move(item), 0);
            else
                results = handler->handleGlobalQuery(this);

            auto d_h = duration_cast<milliseconds>(system_clock::now()-t).count();

            t = system_clock::now();
            handler->applyUsageScore(&results);
            auto d_s = duration_cast<milliseconds>(system_clock::now()-t).count();

            // makes no sense to time this, since waiting for unlock
            unique_lock lock(rank_items_mutex);
            rank_items.reserve(rank_items.size() + results.size());
            for (auto &rank_item : results)
                rank_items.emplace_back(handler, ::move(rank_item));

            qCDebug(timeCat,).noquote()
                << QStringLiteral("\x1b[38;5;244m│%1 ms│%2 ms│%3│ #%4 '%5' %6\x1b[0m")
                       .arg(d_h, 6)
                       .arg(d_s, 6)
                       .arg(results.size(), 6)
                       .arg(query_id)
                       .arg(string_, handler->id());
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
    auto d_h = duration_cast<milliseconds>(system_clock::now()-tp).count();

    static const auto cmp = [](const auto &a, const auto &b){
        if (a.second.score == b.second.score)
            return a.second.item->text() > b.second.item->text();
        else
            return a.second.score > b.second.score;
    };

    tp = system_clock::now();
    auto begin = ::begin(rank_items);
    auto end = ::end(rank_items);
    auto mid = begin + 20;

    // Partially sort the visible items for fast response times
    if (mid < end)
    {
        partial_sort(begin, mid, end, cmp);
        addRankItems(begin, mid);
        begin = mid;
    }

    sort(begin, end, cmp);
    addRankItems(begin, end);

    auto d_s = duration_cast<milliseconds>(system_clock::now()-tp).count();

    qCDebug(timeCat,).noquote() << QStringLiteral("\x1b[38;5;33m│ Handling│  Sorting│ Count│\x1b[0m");

    qCDebug(timeCat,).noquote()
        << QStringLiteral("\x1b[38;5;33m│%1 ms│%2 ms│%3│ #%4 GLOBAL '%5'\x1b[0m")
               .arg(d_h, 6)
               .arg(d_s, 6)
               .arg(rank_items.size(), 6)
               .arg(query_id)
               .arg(string_);
}

void GlobalQuery::addRankItems(vector<pair<Extension*,RankItem>>::iterator begin,
                               vector<pair<Extension*,RankItem>>::iterator end)
{
    unique_lock lock(results_buffer_mutex_);

    for (auto it = begin; it < end; ++it)
        results_buffer_.emplace_back(it->first, ::move(it->second.item));

    if (valid_)
        invokeCollectResults();
}
