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

    ~AsyncExecution() { cancel(); }

    void cancel() override
    {
        generator.reset();
        if (active)
            emit activeChanged(active = false);
    }

    bool isActive() const override { return active; }

    bool canFetchMore() const override {
        return context.isValid()
               && (!iterator
                   // https://github.com/qcoro/qcoro/issues/294
                   || iterator != const_cast<AsyncItemGenerator*>(generator.get())->end());
    }

    void fetchMore() override
    {
        if (!active && canFetchMore())
            fetch_task = fetchMoreTask();
    }

    QCoro::Task<> fetchMoreTask()
    {
        emit activeChanged(active = true);

        try {

            if (iterator == nullopt)
                iterator = co_await generator->begin();
            else
                co_await ++(*iterator);

            if (*iterator != generator->end())
                results.add(::move(**iterator));

        } catch (const exception &e) {
            WARN << u"AsyncGeneratorQueryHandler threw exception:\n"_s << e.what();
        } catch (...) {
            WARN << u"AsyncGeneratorQueryHandler threw unknown exception."_s;
        }

        emit activeChanged(active = false);
    }
};

AsyncGeneratorQueryHandler::~AsyncGeneratorQueryHandler() {}

unique_ptr<QueryExecution> AsyncGeneratorQueryHandler::execution(QueryContext &ctx)
{ return make_unique<AsyncExecution>(ctx, items(ctx)); }
