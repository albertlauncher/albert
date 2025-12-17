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

// -------------------------------------------------------------------------------------------------

class GlobalQueryResult : public albert::RankItem
{
public:
    explicit GlobalQueryResult(albert::GlobalQueryHandler *handler,
                               const albert::RankItem &rank_item) noexcept :
        RankItem(rank_item),
        handler(handler)
    {}
    ~GlobalQueryResult() noexcept {}
    albert::GlobalQueryHandler *handler;
};

// -------------------------------------------------------------------------------------------------

struct MappedData {
    GlobalQueryHandler *handler;
    vector<RankItem> rank_items;
    uint handling_duration;
    uint scoring_duration;
};

//V function(T &result, const U &intermediate)
struct ReducedData {
    struct Diagnostics {
        albert::GlobalQueryHandler *handler;
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
    Private(GlobalQueryExecution *, vector<albert::GlobalQueryHandler *>);

    void addResultChunk();

    GlobalQueryExecution *q;
    const vector<albert::GlobalQueryHandler*> handlers;
    atomic_bool valid;
    bool active;

    QFutureWatcher<ReducedData> future_watcher;

    vector<GlobalQueryResult> unordered_results;
    chrono::time_point<chrono::system_clock> start_timepoint;
    chrono::time_point<chrono::system_clock> finish_timepoint;
};

GlobalQueryExecution::Private::Private(GlobalQueryExecution *execution,
                                       vector<GlobalQueryHandler *> h) :
    q(execution),
    handlers(::move(h)),
    valid(true),
    active(true)
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
                if (q->query.string().isEmpty()) // important redirection
                    for (auto &item : handler->handleEmptyQuery()) // order ???
                        data.rank_items.emplace_back(::move(item), 0);
                else
                    data.rank_items = handler->rankItems(*q);
                data.handling_duration = duration_cast<milliseconds>(system_clock::now()-t).count();

                t = system_clock::now();
                q->usageScoring().modifyMatchScores(handler->id(), data.rank_items);
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
            for (auto &rank_item : mapped.rank_items)
                reduced.results.emplace_back(mapped.handler, rank_item);  // copies, but at least threaded
        }
    );

    QObject::connect(&future_watcher, &QFutureWatcher<ReducedData>::finished, q, [this] {
        if (valid)
        {
            auto reduced = future_watcher.future().takeResult();

            const auto total_duration = duration_cast<milliseconds>(system_clock::now() - start_timepoint).count();

            static const auto header  = color::blue + u"╭ Handling╷  Scoring╷ Count╷"_s + color::reset;
            static const auto body    = color::blue + u"│%1 ms│%2 ms│%3│ #%4 '%5' %6"_s + color::reset;
            static const auto fheader = color::blue + u"├ Handling│         │ Count╷"_s + color::reset;
            static const auto footer  = color::blue + u"╰%1 ms╵         ╵%2╵ #%3 '%4' TOTAL"_s + color::reset;

            DEBG << header;

            for (const auto &diag : reduced.handler_diag)
                DEBG << body
                            .arg(diag.handling_runtime, 6)
                            .arg(diag.scoring_runtime, 6)
                            .arg(diag.item_count, 6)
                            .arg(q->id)
                            .arg(q->string(), diag.handler->id());

            DEBG << fheader;
            DEBG << footer
                        .arg(total_duration, 6)
                        .arg(reduced.results.size(), 6)
                        .arg(q->id)
                        .arg(q->query.string());

            unordered_results = ::move(reduced.results);

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

    ranges::partial_sort(reverse_view, fetch_view.end(), greater{});

    // FIXME ranges::to
    auto take_view = fetch_view | views::transform([](const GlobalQueryResult &r) {
                         return QueryResult(r.handler, ::move(r.item));
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

GlobalQueryExecution::GlobalQueryExecution(Query &q, vector<GlobalQueryHandler*> h)
    : QueryExecution(q)
    , d(make_unique<Private>(this, ::move(h)))
{}

GlobalQueryExecution::~GlobalQueryExecution()
{
    cancel();
    if (!d->future_watcher.isFinished())
    {
        DEBG << QString("Busy wait on query: #%1").arg(id);
        d->future_watcher.waitForFinished();
    }
}

bool GlobalQueryExecution::isValid() const { return d->valid; }

const QueryHandler &GlobalQueryExecution::handler() const { return query.handler(); }

QString GlobalQueryExecution::string() const
{ return query.string() == "*" ? QString() : query.string(); }

QString GlobalQueryExecution::trigger() const { return query.trigger(); }

const albert::UsageScoring &GlobalQueryExecution::usageScoring() const
{ return query.usageScoring(); }

void GlobalQueryExecution::cancel()
{
    if (d->valid)
    {
        d->valid = false;
        if (!d->future_watcher.isFinished())
            d->future_watcher.cancel();
    }
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

