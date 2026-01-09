// Copyright (c) 2023-2024 Manuel Schneider

#include "query.h"
#include "queryexecution.h"
#include "queryhandler.h"
#include "queryresults.h"
#include "usagescoring.h"
#include <memory>
#include <vector>
using namespace albert::detail;
using namespace std;

class Query::Private
{
public:
    UsageScoring usage_scoring;

    atomic_bool valid;
    QueryHandler &handler;
    QString trigger;
    QString string;

    QueryResults matches;
    QueryResults fallbacks;

    std::unique_ptr<QueryExecution> execution;
};

Query::Query(UsageScoring usage_scoring,
             vector<QueryResult> &&fallbacks,
             QueryHandler &handler,
             QString trigger,
             QString string) :
    d(new Private{.usage_scoring = ::move(usage_scoring),
                  .valid = true,
                  .handler = handler,
                  .trigger = trigger,
                  .string = string,
                  .matches = {*this},
                  .fallbacks = {*this},
                  .execution = {}})
{
    d->fallbacks.add(::move(fallbacks));

    // CRUCIAL: Instantiate execution here.
    // Do NOT construct the exection before query instance is constructed completely.
    // `QueryExecution`s use `Query` which has to be valid throughout their entire lifetime.
    // Note: While creating `Private` `Query::d` is not yet assigned.
    d->execution = handler.execution(*this);

    // DEBG << QString("Query created. [#%1 '%2']").arg(d->execution->id).arg(d->string);
}

Query::~Query()
{
    d->valid = false;

    // DEBG << QString("Query about to be deleted. [#%1 '%2']").arg(d->execution->id).arg(d->string);

    // If not deleted early, Query::d is under destruction while destructing Query::execution.
    d->execution.reset();
}

const albert::UsageScoring &Query::usageScoring() const { return d->usage_scoring; }

QString Query::trigger() const { return d->trigger; }

QString Query::query() const { return d->string; }

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
