// Copyright (c) 2022-2025 Manuel Schneider

#include "color.h"
#include "globalqueryexecution.h"
#include "globalqueryhandler.h"
#include "logging.h"
#include "rankitem.h"
#include "usagescoring.h"
#include <QFutureWatcher>
#include <QtConcurrentMap>
#include <chrono>
#include <ranges>
#include <vector>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std::chrono;
using namespace std;

struct GlobalQueryResult
{
    GlobalQueryHandler *handler;
    RankItem rank_item;
};

struct ExecutionContext
{
    GlobalQueryHandler *handler;
    vector<RankItem> rank_items;
    uint handling_duration;
    uint scoring_duration;
};

class GlobalQueryExecution::Private
{
public:
    Private(GlobalQueryExecution *, vector<albert::GlobalQueryHandler *>);

    void addResultChunk();

    GlobalQueryExecution *q;
    vector<ExecutionContext> execution_contexts;
    bool active;
    QFutureWatcher<void> future_watcher;
    vector<GlobalQueryResult> unordered_results;
};

GlobalQueryExecution::Private::Private(GlobalQueryExecution *execution,
                                       vector<GlobalQueryHandler*> h) :
    q(execution),
    execution_contexts(h.size()),
    active(true)
{
    for (auto i = 0u; i < h.size(); ++i)
        execution_contexts[i].handler = h[i];

    auto future = QtConcurrent::map(execution_contexts, [this](ExecutionContext &ctx) {
        try {
            auto t = system_clock::now();
            if (q->context.query().isEmpty()) // important redirection
                for (auto &item : ctx.handler->handleEmptyQuery()) // order ???
                    ctx.rank_items.emplace_back(::move(item), 0);
            else
                ctx.rank_items = ctx.handler->rankItems(*q);
            ctx.handling_duration = duration_cast<milliseconds>(system_clock::now()-t).count();

            t = system_clock::now();
            q->usageScoring().modifyMatchScores(ctx.handler->id(), ctx.rank_items);
            ctx.scoring_duration = duration_cast<milliseconds>(system_clock::now()-t).count();
        }
        catch (const exception &e) {
            WARN << u"GlobalQueryHandler '%1' threw exception:\n"_s.arg(ctx.handler->id()) << e.what();
        }
        catch (...) {
            WARN << u"GlobalQueryHandler '%1' threw unknown exception:\n"_s.arg(ctx.handler->id());
        }
    });

    QObject::connect(&future_watcher, &QFutureWatcher<void>::finished,
                     q, [this, tp=system_clock::now()]
    {
        if (q->isValid())
        {
            static const auto header = color::blue + u"╭ Handling╷  Scoring╷ Count╷ Query #%1 '%2'"_s + color::reset;
            DEBG << header.arg(q->id).arg(q->context.query());

            static const auto body = color::blue + u"│%1 ms│%2 ms│%3│ %4"_s + color::reset;
            for (const auto &ctx : execution_contexts)
                DEBG << body.arg(ctx.handling_duration, 6)
                            .arg(ctx.scoring_duration, 6)
                            .arg(ctx.rank_items.size(), 6)
                            .arg(ctx.handler->id());

            auto total = 0;
            for (const auto &ec : execution_contexts)
                total += ec.rank_items.size();
            unordered_results.reserve(total);
            for (auto &ctx : execution_contexts)
                for (auto &rank_item : ctx.rank_items)
                    unordered_results.emplace_back(ctx.handler, ::move(rank_item));

            static const auto footer = color::blue + u"╰%1 ms╵         ╵%2╵ TOTAL"_s + color::reset;
            DEBG << footer
                        .arg(duration_cast<milliseconds>(system_clock::now() - tp).count(), 6)
                        .arg(unordered_results.size(), 6);

            execution_contexts.clear();

            // Required because while active fetchMore has no effect
            addResultChunk();
        }

        emit q->activeChanged(active = false);
    });

    future_watcher.setFuture(future);
}

void GlobalQueryExecution::Private::addResultChunk()
{
    auto tp = system_clock::now();

    // Partial sort the items incrementally in reverse order (for cheap "pop_n")
    auto reverse_view = unordered_results | views::reverse;

    auto fetch_view = reverse_view | views::take(10);

    ranges::partial_sort(reverse_view, fetch_view.end(), greater{}, &GlobalQueryResult::rank_item);

    // FIXME ranges::to
    auto take_view = fetch_view | views::transform([](GlobalQueryResult &r) {
                         return QueryResult(r.handler, ::move(r.rank_item.item));
                     });

    vector<QueryResult> taken{begin(take_view), end(take_view)};

    // Cheap pop_n
    unordered_results.erase(unordered_results.end() - fetch_view.size(), unordered_results.end());

    const auto duration_sort = duration_cast<milliseconds>(system_clock::now() - tp).count();
    DEBG << u"Fetched %1 items in %2 ms"_s.arg(taken.size()).arg(duration_sort);

    // Query::add emits model signals that may lead to fetchMore recursions.
    // Ensure unfetched_rank_items integrity _before adding_!
    q->results.add(::move(taken));
}

// -------------------------------------------------------------------------------------------------

GlobalQueryExecution::GlobalQueryExecution(QueryContext &c, vector<GlobalQueryHandler*> h)
    : QueryExecution(c)
    , d(make_unique<Private>(this, ::move(h)))
{}

GlobalQueryExecution::~GlobalQueryExecution()
{
    cancel();

    // Qt 6.4 QFutureWatcher is broken.
    // isFinished returns wrong values and waitForFinished blocks forever on finished futures.
    // TODO(26.04): Remove workaround when dropping Qt < 6.5 support.
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (!d->future_watcher.isFinished())
#else
    if (d->future_watcher.isRunning())
#endif
    {
        DEBG << QString("Busy wait on query: #%1").arg(id);
        d->future_watcher.waitForFinished();
    }
}

bool GlobalQueryExecution::isValid() const { return context.isValid(); }

const QueryHandler &GlobalQueryExecution::handler() const { return context.handler(); }

QString GlobalQueryExecution::query() const
{ return context.query() == "*" ? QString() : context.query(); }

QString GlobalQueryExecution::trigger() const { return context.trigger(); }

const albert::UsageScoring &GlobalQueryExecution::usageScoring() const
{ return context.usageScoring(); }

void GlobalQueryExecution::cancel()
{
    disconnect(&d->future_watcher, &QFutureWatcher<void>::finished, this, nullptr);
    d->future_watcher.cancel();
}

void GlobalQueryExecution::fetchMore()
{
    if (!isActive() && canFetchMore())
    {
        emit activeChanged(d->active = true);
        d->addResultChunk();
        emit activeChanged(d->active = false);
    }
}

bool GlobalQueryExecution::canFetchMore() const { return !d->unordered_results.empty(); }

bool GlobalQueryExecution::isActive() const { return d->active; }

