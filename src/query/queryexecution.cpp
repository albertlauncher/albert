// Copyright (c) 2022-2025 Manuel Schneider

#include "queryexecution.h"
using namespace albert;

namespace {
uint query_execution_count = 0;
}

QueryExecution::QueryExecution(albert::Query &q)
    : id(query_execution_count++)
    , query(q)
    , results(q)
{}

