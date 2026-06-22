// Copyright (c) 2023-2026 Manuel Schneider

#include "querycontext.h"
#include "query.h"
using namespace albert;
using namespace std;

QueryContext::QueryContext(const detail::Query &q) :
    query_(q)
{}

bool QueryContext::isValid() const { return query_.valid_; }

uint QueryContext::id() const { return query_.id_; }

const QString &QueryContext::trigger() const { return query_.trigger_; }

const QString &QueryContext::query() const { return query_.query_; }

const QueryHandler &QueryContext::handler() const { return query_.handler_; }

const UsageScoring &QueryContext::usageScoring() const { return query_.usage_scoring_; }
