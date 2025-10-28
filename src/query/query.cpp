// Copyright (c) 2023-2024 Manuel Schneider

#include "fallbackhandler.h"
#include "logging.h"
#include "queryengine.h"
#include "queryexecution.h"
#include "queryhandler.h"
#include "queryprivate.h"
#include "queryresults.h"
#include "rankitem.h"
#include <memory>
#include <ranges>
#include <vector>
using namespace albert::detail;
using namespace std;

class Query::Private
{
public:
    QueryEngine &query_engine;

    atomic_bool valid;
    QueryHandler &handler;
    QString trigger;
    QString string;

    QueryResults matches;
    QueryResults fallbacks;

    std::unique_ptr<QueryExecution> execution;
};

Query::Query(QueryEngine &query_engine,
             QueryHandler &handler,
             QString trigger,
             QString string) :
    d(new Private{.query_engine=query_engine,
                  .valid = true,
                  .handler = handler,
                  .trigger = trigger,
                  .string = string,
                  .matches = {*this},
                  .fallbacks = {*this},
                  .execution = {}})
{
    if (!(trigger.isEmpty() && string.isEmpty()))  // TODO redesign fallbacks
    {
        const auto &order = d->query_engine.fallbackOrder();

        vector<pair<FallbackHandler*, RankItem>> fallbacks;

        for (auto &[id, fallback_handler] : d->query_engine.fallbackHandlers())
            for (auto item : fallback_handler->fallbacks(d->trigger + d->string))
                if (auto it = order.find(make_pair(id, item->id()));
                    it == order.end())
                    fallbacks.emplace_back(fallback_handler, RankItem(::move(item), 0));
                else
                    fallbacks.emplace_back(fallback_handler, RankItem(::move(item), it->second));

        ranges::sort(fallbacks, greater(), &decltype(fallbacks)::value_type::second);

        d->fallbacks.add(fallbacks | views::transform([](auto &p) {
                             return QueryResult{p.first, ::move(p.second.item)};
                         }));
    }

    // CRUCIAL: Instantiate execution here.
    // Do NOT construct the exection before query instance is constructed completely.
    // `QueryExecution`s use `Query` which has to be valid throughout their entire lifetime.
    // Note: While creating `Private` `Query::d` is not yet assigned.
    d->execution = handler.execution(*this);

    // DEBG << QString("Query created. [#%1 '%2']").arg(d->execution->id).arg(d->string);
}

Query::~Query()
{
    // DEBG << QString("Query about to be deleted. [#%1 '%2']").arg(d->execution->id).arg(d->string);

    // If not deleted early, Query::d is under destruction while destructing Query::execution.
    d->execution.reset();
}

QString Query::trigger() const { return d->trigger; }

QString Query::string() const { return d->string; }

albert::QueryResults &Query::matches() { return d->execution->results; }

albert::QueryResults &Query::fallbacks() { return d->fallbacks; }

albert::QueryHandler &Query::handler() const { return d->handler; }

albert::QueryExecution &Query::execution() const { return *d->execution; }

bool Query::isValid() const { return d->valid; }

void Query::cancel()
{
    if (d->valid)
    {
        d->valid = false;
        d->execution->cancel();
    }
}
