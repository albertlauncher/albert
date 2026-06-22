// Copyright (c) 2023-2026 Manuel Schneider

#include "query.h"
#include "logging.h"
#include "queryexecution.h"
#include "queryhandler.h"
#include "queryresults.h"
#include "usagescoring.h"
#include <memory>
#include <vector>
#include <chrono>
using namespace albert::detail;
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;
using namespace std::chrono;

namespace {
uint query_count = 0;
}

Query::Query(const QString &trigger,
             const QString &query,
             QueryHandler &handler,
             vector<QueryResult> fallbacks,
             UsageScoring usage_scoring) :
    id_(++query_count),
    trigger_(trigger),
    query_(query),
    synopsis_(handler.synopsis(query)),  // may throw
    valid_(true),
    usage_scoring_(::move(usage_scoring)),
    handler_(handler),
    execution_(handler.execution(*this)),  // may throw
    is_active_(false)
{
    fallbacks_.add(::move(fallbacks));
    connect(execution_.get(), &QueryExecution::fetchFinished,
            this, [this] { emit activeChanged(is_active_ = false); });
}

Query::~Query() { cancel(); }

QString Query::trigger() const { return trigger_; }

QString Query::query() const { return query_; }

QString Query::synopsis() const { return synopsis_; }

QueryHandler &Query::handler() const {return handler_; }

bool Query::isValid() const { return valid_; }

bool Query::isActive() const { return is_active_; }

bool Query::canFetchMore() const
{
    if (!valid_ || is_active_)
        return false;

    try
    {
        return execution_->canFetchMore();
    }
    catch (const exception &e)
    {
        WARN << u"QueryHandler::canFetchMore threw:\n"_s << e.what();
    }
    catch (...)
    {
        WARN << u"QueryHandler::canFetchMore threw unknown exception."_s;
    }
    return false;
}

void Query::fetchMore()
{
    if (!valid_ || is_active_)
        return;

    DEBG << u"Fetch started (#%1)"_s.arg(id_);

    connect(execution_.get(), &QueryExecution::fetchFinished,
            this, [this, tp=system_clock::now()] {
                DEBG << u"Fetch finshed (#%1) after %2 ms"_s
                            .arg(id_)
                            .arg(duration_cast<milliseconds>(system_clock::now() - tp).count());
            }, Qt::SingleShotConnection);

    emit activeChanged(is_active_ = true);

    try
    {
        execution_->fetchMore();
    }
    catch (const exception &e)
    {
        WARN << u"QueryHandler::fetchMore threw:\n"_s << e.what();
    }
    catch (...)
    {
        WARN << u"QueryHandler::fetchMore threw unknown exception."_s;
    }
}

void Query::cancel()
{
    if (!valid_)
        return;

    valid_ = false;

    if (!is_active_)
        return;

    try
    {
        execution_->cancel();
    }
    catch (const exception &e)
    {
        WARN << u"QueryHandler::cancel threw:\n"_s << e.what();
    }
    catch (...)
    {
        WARN << u"QueryHandler::cancel threw unknown exception."_s;
    }
}

QueryResults &Query::matches() { return execution_->results; }

QueryResults &Query::fallbacks() { return fallbacks_; }
