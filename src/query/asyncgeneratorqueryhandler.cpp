// Copyright (c) 2023-2025 Manuel Schneider

#include "asyncgeneratorqueryhandler.h"
#include "logging.h"
#include "queryexecution.h"
#include <QCoroAsyncGenerator>
#include <QCoroTask>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

class AsyncExecution final : public QueryExecution
{
    unique_ptr<AsyncItemGenerator> generator;
    optional<AsyncItemGenerator::iterator> iterator;
    QCoro::Task<> fetch_task;
    bool active;

public:

    AsyncExecution(QueryContext &ctx, AsyncItemGenerator &&gen)
        : QueryExecution(ctx)
        , generator(make_unique<AsyncItemGenerator>(::move(gen)))
        , iterator(nullopt)
        , active(false)
    {
        fetchMore();
    }

    ~AsyncExecution()
    {
        // ~fetch_task deletes the suspended coro awaiting the generator
        // ~generator deletes the suspended generator coro. Unwinding the frame may block.
    }

    void cancel() override
    {
        // Deleting the generator, forces coroutine frame destruction and as such MAY BLOCK because
        // the coroutine implementation deliberately does so, e.g. to join a thread or such.
        // Since it must not block, deleting the generator on cancel is not an option.
    }

    bool isActive() const override { return active; }

    bool canFetchMore() const override {
        return context.isValid()
               && (!iterator
                   // https://github.com/qcoro/qcoro/issues/294
                   || *iterator != const_cast<AsyncItemGenerator&>(*generator).end());
    }

    void fetchMore() override
    {
        if (!active && canFetchMore())
            fetch_task = fetchMoreTask();
    }

    QCoro::Task<> fetchMoreTask()
    {
        struct ScopedActivation {
            AsyncExecution &exec;
            ScopedActivation(AsyncExecution &e) : exec(e)
            { emit exec.activeChanged(exec.active = true); }
            ~ScopedActivation()
            { emit exec.activeChanged(exec.active = false); }
        } scoped_activation(*this);

        try {

            if (iterator)
                co_await ++(*iterator);
            else
                iterator = co_await generator->begin();

            if (*iterator != generator->end())
                results.add(::move(**iterator));

        } catch (const exception &e) {
            WARN << u"AsyncGeneratorQueryHandler threw exception:\n"_s << e.what();
        }
        catch (...)
        {
            WARN << u"AsyncGeneratorQueryHandler threw unknown exception."_s;
        }
    }
};

AsyncGeneratorQueryHandler::~AsyncGeneratorQueryHandler() {}

unique_ptr<QueryExecution> AsyncGeneratorQueryHandler::execution(QueryContext &ctx)
{ return make_unique<AsyncExecution>(ctx, items(ctx)); }
