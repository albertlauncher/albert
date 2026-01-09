// Copyright (c) 2022-2025 Manuel Schneider

#include "queryexecution.h"
using namespace albert;

namespace {
uint query_execution_count = 0;
}

QueryExecution::QueryExecution(QueryContext &ctx)
    : id(query_execution_count++)
    , context(ctx)
    , results(ctx)
{}

