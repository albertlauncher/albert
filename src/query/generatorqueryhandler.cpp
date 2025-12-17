// Copyright (c) 2023-2025 Manuel Schneider

#include "generatorqueryhandler.h"
#include "logging.h"
#include <QCoroGenerator>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <albert/queryexecution.h>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

GeneratorQueryHandler::~GeneratorQueryHandler() {}

class GeneratorQueryHandlerExecution final : public QueryExecution
{
    QFutureWatcher<void> watcher;  // implicit active flag
    ItemGenerator generator;  // mutexed
    optional<ItemGenerator::iterator> iterator;  // mutexed
    bool active;
    // begin and operator++ are potentially long blocking operations.
    // it had to be mutexed because canFetchMore may check the iterator in the main thread.
    // awaiting the lock however blocks the main thread potentially long.
    // store a simple atomic at_end flag to avoid this.
    // now theres only generator and iterator left that are touched in the thread
    // due to the active flag they will never run concurrently
    // so we dont actually need to mutex them at all
    atomic_bool at_end;

public:

    GeneratorQueryHandlerExecution(QueryContext &ctx, ItemGenerator gen)
        : QueryExecution(ctx)
        , generator(::move(gen))
        , iterator(nullopt)
        , active(false)
        , at_end(false)
    {
        connect(&watcher, &QFutureWatcher<void>::finished,
                this, &GeneratorQueryHandlerExecution::onFetchFinished);
        fetchMore();
    }

    ~GeneratorQueryHandlerExecution()
    {
        cancel();
        if (!watcher.isFinished())
        {
            DEBG << QString("Busy wait on query: #%1").arg(id);
            watcher.waitForFinished();
        }
    }

    void cancel() override { }

    bool isActive() const override { return active; }

    bool canFetchMore() const override { return context.isValid() && !at_end; }

    void fetchMore() override
    {
        if (!isActive() && canFetchMore())
        {
            emit activeChanged(active = true);
            watcher.setFuture(QtConcurrent::run([this]()
            {
                if (context.isValid())
                {
                    try {
                        if (iterator)
                            ++iterator.value();
                        else
                            iterator = generator.begin();
                        at_end = iterator == generator.end();
                    } catch (const exception &e) {
                        WARN << u"GeneratorQueryHandler (next) threw exception:\n"_s << e.what();
                    } catch (...) {
                        WARN << u"GeneratorQueryHandler (next) threw unknown exception."_s;
                    }
                }
            }));
        }
    }

    void onFetchFinished()
    {
        if (context.isValid())
        {
            std::vector<std::shared_ptr<albert::Item>> items;
            try {
                if (iterator && *iterator != generator.end())
                    items = ::move(**iterator);
            } catch (const exception &e) {
                WARN << u"GeneratorQueryHandler (yield) threw exception:\n"_s << e.what();
            } catch (...) {
                WARN << u"GeneratorQueryHandler (yield) threw unknown exception."_s;
            }
            results.add(::move(items));
        }

        emit activeChanged(active = false);
    }
};

unique_ptr<QueryExecution> GeneratorQueryHandler::execution(QueryContext &ctx)
{ return make_unique<GeneratorQueryHandlerExecution>(ctx, items(ctx)); }


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
