// Copyright (c) 2022-2025 Manuel Schneider

#include "globalquery.h"
#include "globalqueryexecution.h"
#include <ranges>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

QString GlobalQuery::id() const { return u"globalquery"_s; }

QString GlobalQuery::name() const { return u"Global query"_s; }

QString GlobalQuery::description() const { return u"Runs a bunch of global query handlers"_s; }

unique_ptr<QueryExecution> GlobalQuery::execution(QueryContext &ctx)
{ return make_unique<GlobalQueryExecution>(ctx, handlers | views::values | ranges::to<vector>()); }

QString GlobalQuery::synopsis(const QString &query) const
{ return query == u"*"_s ? u"🕚"_s : u""_s; }
