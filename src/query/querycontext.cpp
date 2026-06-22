// Copyright (c) 2023-2026 Manuel Schneider

#include "querycontext.h"
#include "query.h"
using namespace albert;
using namespace std;

QueryContext::QueryContext(const detail::Query &q) :
    query_(q)
{}

bool QueryContext::isValid() const { return query_.isValid(); }

uint QueryContext::id() const { return query_.id(); }

const QString &QueryContext::trigger() const { return query_.trigger(); }

const QString &QueryContext::query() const { return query_.query(); }

const QueryHandler &QueryContext::handler() const { return query_.handler(); }

const UsageScoring &QueryContext::usageScoring() const { return query_.usageScoring(); }
