// Copyright (c) 2023-2025 Manuel Schneider

#include "asyncgeneratorqueryhandler.h"
#include "logging.h"
#include "queryexecution.h"
#include <QCoroAsyncGenerator>
#include <QCoroTask>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

class AsyncGeneratorQueryExecution final : public QueryExecution
{
    AsyncGeneratorQueryHandler &handler;
    AsyncItemGenerator generator;
    optional<AsyncItemGenerator::iterator> iterator;
    optional<QCoro::Task<>> fetch_task;

public:
    AsyncGeneratorQueryExecution(AsyncGeneratorQueryHandler &h, QueryContext c) :
        handler(h),
        generator(h.items(c))
    {}

    ~AsyncGeneratorQueryExecution()
    {
        // ~fetch_task deletes the suspended coro awaiting the generator
        // ~generator deletes the suspended generator coro. Unwinding the frame may block.
    }

    // https://github.com/qcoro/qcoro/issues/294
    bool canFetchMore() const override
    { return !iterator || *iterator != const_cast<AsyncItemGenerator&>(generator).end(); }

    void fetchMore() override
    {
        fetch_task = [this] -> QCoro::Task<> {
            try {
                if (iterator = iterator ? co_await ++(*iterator) : co_await generator.begin();
                    *iterator != generator.end())
                    results.add(handler, ::move(**iterator));
            }
            catch (const exception &e) {
                throw;
            }
            catch (...) {
                throw runtime_error("Unknown exception.");
            }
        }()
        .then(
            [this]{
                emit fetchFinished();
            },
            [this](const exception &e) {
                WARN << u"AsyncGeneratorQueryHandler threw exception:\n"_s << e.what();
                iterator = generator.end();
                emit fetchFinished();
            }
        );
    }

    void cancel() override
    {
        // Deleting the generator, forces coroutine frame destruction and as such MAY BLOCK because
        // the coroutine implementation deliberately does so, e.g. to join a thread or such.
        // Since it must not block, deleting the generator on cancel is not an option.
    }
};

AsyncGeneratorQueryHandler::~AsyncGeneratorQueryHandler() {}

unique_ptr<QueryExecution> AsyncGeneratorQueryHandler::execution(QueryContext ctx)
{ return make_unique<AsyncGeneratorQueryExecution>(*this, ctx); }
