// Copyright (c) 2023-2025 Manuel Schneider

#include "generatorqueryhandler.h"
#include "logging.h"
#include "usagescoring.h"
#include <QCoroGenerator>
#include <QFuture>
#include <QtConcurrentRun>
#include <albert/queryexecution.h>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

GeneratorQueryHandler::~GeneratorQueryHandler() {}

class GeneratorQueryHandlerExecution final : public QueryExecution
{
    GeneratorQueryHandler &handler;
    QueryContext context;
    optional<ItemGenerator> generator;
    optional<ItemGenerator::iterator> iterator;
    QFuture<void> future;
    // items(), begin and operator++ are potentially long blocking operations.
    // it had to be mutexed because canFetchMore may check the iterator in the main thread.
    // awaiting the lock however blocks the main thread potentially long.
    // store a simple atomic at_end flag to avoid this.
    // now theres only generator and iterator left that are touched in the thread
    // due to the active flag they will never run concurrently
    // so we dont actually need to mutex them at all
    atomic_bool at_end;

public:
    GeneratorQueryHandlerExecution(GeneratorQueryHandler &h, QueryContext c) :
        handler(h),
        context(c),
        at_end(false)
    {}

    ~GeneratorQueryHandlerExecution()
    {
        // Qt 6.4 QFuture is broken.
        // isFinished returns wrong values and waitForFinished blocks forever on finished futures.
        // TODO(26.04): Remove workaround when dropping Qt < 6.5 support.
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        if (!future.isFinished())
#else
        if (future.isRunning())
#endif
        {
            DEBG << QString("Busy wait on query: #%1").arg(context.id());
            future.waitForFinished();
        }
    }

    bool canFetchMore() const override { return !at_end; }

    void fetchMore() override
    {
        future = QtConcurrent::run([this] -> vector<shared_ptr<Item>>
        {
            // `items()` could also be a regular function that returns a generator.
            // This function should as well run in the thread.
            if (!generator)
                generator = handler.items(context);

            if (iterator)
                ++*iterator;
            else
                iterator = generator->begin();

            if (iterator != generator->end())
                return ::move(*iterator.value());

            at_end = true;
            return {};
        })
        .then(this, [this](vector<shared_ptr<Item>> items){
            if (!context.isValid())
                return;
            results.add(handler, ::move(items));
        })
        .onFailed(this, [](const QUnhandledException &que) {
            if (que.exception())
                rethrow_exception(que.exception());
            else
                throw runtime_error("QUnhandledException::exception() returned nullptr.");
        })
        .onFailed(this, [](const exception &e) {
            WARN << u"GeneratorQueryHandler threw exception:\n"_s << e.what();
        })
        .onFailed(this, [] {
            WARN << u"GeneratorQueryHandler threw unknown exception."_s;
        })
        .then(this, [this]{
            emit fetchFinished();
        });
    }

    void cancel() override { }
};

unique_ptr<QueryExecution> GeneratorQueryHandler::execution(QueryContext ctx)
{ return make_unique<GeneratorQueryHandlerExecution>(*this, ctx); }

ItemGenerator GeneratorQueryHandler::lazySort(vector<RankItem> rank_items)
{
    while(!rank_items.empty())
    {
        // Partial sort the items incrementally in reverse order (for cheap "pop_n")
        auto reverse_view = rank_items | views::reverse;
        auto take_view = reverse_view | views::take(10);
        ranges::partial_sort(reverse_view, take_view.end(), greater{});

        // Yield chunk
        auto item_view = take_view | views::transform(&RankItem::item);
        vector<shared_ptr<Item>> item_vector {
            make_move_iterator(begin(item_view)),
            make_move_iterator(end(item_view))
        };

        // Cheap pop_n
        rank_items.erase(rank_items.end() - take_view.size(),rank_items.end());

        co_yield ::move(item_vector);
    }
}

ItemGenerator GeneratorQueryHandler::lazySort(vector<RankItem> items,
                                              const UsageScoring &scoring) const
{ return lazySort(scoring.applied(id(), ::move(items))); }

// -------------------------------------------------------------------------------------------------
// Future queryhandler implementation. Based on AsyncGeneratorQueryHandler.
// -------------------------------------------------------------------------------------------------

// // This type is required because deleting coroutines simply cleans up the stack frame and QCoro does
// // not provide any kind of clean up facilities. When not making sure to wait for the threads to
// // finish before the stack frame is unwound segfaults may appear due to the thread accessing already
// // freed memory. So this class makes sure bind the lifetime of the coroutine to the lifetime of the
// // thread.
// template<typename T>
// struct BlockingFutureDeleter {
//     void operator()(QFutureWatcher<T>* watcher)
//     {
//         if (watcher)
//         {
//             if (!watcher->isFinished())
//                 watcher->waitForFinished();
//             watcher->~QFutureWatcher<T>();
//         }
//     }
// };

// AsyncItemGenerator GeneratorQueryHandler::asyncItemGenerator(Query &query)
// {
//     ItemGenerator sync_gen = itemGenerator(query);
//     ItemGenerator::iterator it = sync_gen.end();
//     unique_ptr<QFutureWatcher<void>, BlockingFutureDeleter<void>> watcher(new QFutureWatcher<void>());

//     struct V
//     {Query &query;
//         V(Query &query) :
//             query(query)
//         {}
//         ~V() { CRIT << "Destroying asyncItemGenerator coroutine for query " << query.string(); }
//     } v(query);

//     auto task = qCoro(watcher.get(), &QFutureWatcher<void>::finished);

//     // https://github.com/qcoro/qcoro/issues/312
//     watcher->setFuture(QtConcurrent::run([&] {


//         CRIT << "sync_gen.begin()";

// it = sync_gen.begin(); }));
//     co_await task;

//     while (it != sync_gen.end()) {
//         auto items =  ::move(*it);
//         CRIT << "Yielding batch of size" << items.size() << "for query " << query.string();
//         co_yield ::move(items);

//         // https://github.com/qcoro/qcoro/issues/312
//         watcher->setFuture(QtConcurrent::run([&] {

//         CRIT << "it++";
//             ++it; }));
//         co_await task;
//     }
//     CRIT << "END asyncItemGenerator";
// }
