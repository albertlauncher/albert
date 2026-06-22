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

struct MappedData {
    GlobalQueryHandler *handler;
    mutable vector<RankItem> rank_items;
    uint handling_duration;
    uint scoring_duration;
};

//V function(T &result, const U &intermediate)
struct ReducedData {
    struct Diagnostics {
        GlobalQueryHandler *handler;
        uint handling_runtime = 0;
        uint scoring_runtime = 0;
        uint item_count = 0;
    };
    vector<Diagnostics> handler_diag;
    vector<GlobalQueryResult> results;
};

class GlobalQueryExecution::Private
{
public:
    Private(GlobalQueryExecution*, QueryContext, vector<GlobalQueryHandler*>);

    void addResultChunk();

    GlobalQueryExecution *q;
    QueryContext context;
    const vector<GlobalQueryHandler*> handlers;

    QFutureWatcher<ReducedData> future_watcher;

    vector<GlobalQueryResult> unordered_results;
    chrono::time_point<chrono::system_clock> start_timepoint;
    chrono::time_point<chrono::system_clock> finish_timepoint;
};

GlobalQueryExecution::Private::Private(GlobalQueryExecution *e,
                                       QueryContext c,
                                       vector<GlobalQueryHandler *> h) :
    q(e),
    context(c),
    handlers(::move(h))
{
    start_timepoint = system_clock::now();

    auto future = QtConcurrent::mappedReduced(
        handlers,
        [this](GlobalQueryHandler *handler) -> MappedData {
            // 6.4 Still no move semantics in QtConcurrent
            MappedData data{.handler = handler,
                            .rank_items = {},
                            .handling_duration = 0,
                            .scoring_duration = 0};
            try {
                auto t = system_clock::now();
                if (context.query().isEmpty()) // important redirection
                    for (auto &item : handler->handleEmptyQuery()) // order ???
                        data.rank_items.emplace_back(::move(item), 0);
                else
                    data.rank_items = handler->rankItems(context);
                data.handling_duration = duration_cast<milliseconds>(system_clock::now()-t).count();

                t = system_clock::now();
                data.rank_items = context.usageScoring().applied(handler->id(), ::move(data.rank_items));
                data.scoring_duration = duration_cast<milliseconds>(system_clock::now()-t).count();
            }
            catch (const exception &e) {
                WARN << u"GlobalQueryHandler '%1' threw exception:\n"_s.arg(handler->id()) << e.what();
            }
            catch (...) {
                WARN << u"GlobalQueryHandler '%1' threw unknown exception:\n"_s.arg(handler->id());
            }

            return data;
        },
        [](ReducedData &reduced, const MappedData &mapped) {
            reduced.handler_diag.emplace_back(mapped.handler,
                                              mapped.handling_duration,
                                              mapped.scoring_duration,
                                              mapped.rank_items.size());
            reduced.results.reserve(reduced.results.size() + mapped.rank_items.size());
            for (auto &&rank_item : mapped.rank_items)
                reduced.results.emplace_back(mapped.handler, ::move(rank_item));
        }
    );

    QObject::connect(&future_watcher, &QFutureWatcher<ReducedData>::finished, q, [this] {
        if (context)
        {
            auto reduced = future_watcher.future().takeResult();

            const auto total_duration = duration_cast<milliseconds>(system_clock::now() - start_timepoint).count();

            static const auto header  = color::blue + u"╭ Handling╷  Scoring╷ Count╷ Query #%1 '%2'"_s + color::reset;
            static const auto body    = color::blue + u"│%1 ms│%2 ms│%3│ %4"_s + color::reset;
            static const auto footer  = color::blue + u"╰%1 ms╵         ╵%2╵ TOTAL"_s + color::reset;

            DEBG << header.arg(context.id()).arg(context.query());
            for (const auto &diag : reduced.handler_diag)
                DEBG << body.arg(diag.handling_runtime, 6)
                            .arg(diag.scoring_runtime, 6)
                            .arg(diag.item_count, 6)
                            .arg(diag.handler->id());
            DEBG << footer.arg(total_duration, 6).arg(reduced.results.size(), 6);

            unordered_results = ::move(reduced.results);

            // Required because while active fetchMore has no effect
            addResultChunk();
        }

        emit q->fetchFinished();
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

GlobalQueryExecution::GlobalQueryExecution(QueryContext c, vector<GlobalQueryHandler *> h) :
    d(make_unique<Private>(this, c, ::move(h)))
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
        DEBG << QString("Busy wait on query: #%1").arg(d->context.id());
        d->future_watcher.waitForFinished();
    }
}

void GlobalQueryExecution::cancel()
{
    disconnect(&d->future_watcher, &QFutureWatcher<ReducedData>::finished, this, nullptr);
    d->future_watcher.cancel();
}

void GlobalQueryExecution::fetchMore()
{
    d->addResultChunk();
    emit fetchFinished();
}

bool GlobalQueryExecution::canFetchMore() const { return !d->unordered_results.empty(); }
