// Copyright (c) 2023-2025 Manuel Schneider

#include "color.h"
#include "logging.h"
#include "queryresults.h"
#include "threadedqueryexecution.h"
#include <QtConcurrentRun>
#include <chrono>
#include <mutex>
#include <ranges>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std::chrono;
using namespace std;

ThreadedQueryExecution::ThreadedQueryExecution(Query &q, ThreadedQueryHandler &h)
    : QueryExecution(q)
    , handler_(h)
    , valid(true)
    , active(true)
{
    future = QtConcurrent::run([this] {
        try
        {
            const auto tp = system_clock::now();
            handler_.handleThreadedQuery(*this);
            const auto duration = duration_cast<milliseconds>(system_clock::now() - tp).count();

            static const auto fmt = color::blue + u"│%1 ms│ TRIGGER │ #%2 '%3'"_s + color::reset;
            DEBG << fmt.arg(duration, 6).arg(id).arg(string());
        }
        catch (const exception &e)
        {
            WARN << u"QueryHandler '%1' threw exception:\n"_s.arg(handler_.id()) << e.what();
        }
        catch (...)
        {
            WARN << u"QueryHandler '%1' threw unknown exception."_s.arg(handler_.id());
        }
    })
    .then(this, [this] { emit activeChanged(active = false); });
}

ThreadedQueryExecution::~ThreadedQueryExecution()
{
    cancel();
    if (future.isRunning()) {
        WARN << QString("Busy wait on query: #%1").arg(id);
        future.waitForFinished();
    }
}

void ThreadedQueryExecution::cancel() { valid = false; }

void ThreadedQueryExecution::fetchMore() {}

bool ThreadedQueryExecution::canFetchMore() const { return false; }

bool ThreadedQueryExecution::isActive() const { return active; }

bool ThreadedQueryExecution::isValid() const { return valid; }

const QueryHandler &ThreadedQueryExecution::handler() const { return query.handler(); }

QString ThreadedQueryExecution::string() const { return query.string(); }

QString ThreadedQueryExecution::trigger() const { return query.trigger(); }

const UsageScoring &ThreadedQueryExecution::usageScoring() const { return query.usageScoring(); }

lock_guard<mutex> ThreadedQueryExecution::getLock() { return lock_guard(match_buffer_mutex); }

vector<shared_ptr<Item>> &ThreadedQueryExecution::matches() { return match_buffer; }

void ThreadedQueryExecution::collect()
{
    // BlockingQueuedConnection makes sure that the results are collected while active is true
    // as a side effect this makes mutexing unnecessary
    QMetaObject::invokeMethod(this,
                              &ThreadedQueryExecution::collectInMainThread,
                              Qt::BlockingQueuedConnection);
}

void ThreadedQueryExecution::collectInMainThread()
{
    if (isValid())
    {
        if (auto lock = getLock(); !match_buffer.empty())
        {
            results.add(::move(match_buffer)
                        | views::transform([this](shared_ptr<Item> &item)
                                           { return QueryResult(&handler_, ::move(item)); }));
            match_buffer.clear();
        }
    }
}
