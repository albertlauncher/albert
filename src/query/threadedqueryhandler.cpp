// Copyright (c) 2023-2025 Manuel Schneider

#include "threadedqueryexecution.h"
#include "threadedqueryhandler.h"
using namespace albert;
using namespace std;

unique_ptr<QueryExecution> ThreadedQueryHandler::execution(Query &query)
{ return make_unique<ThreadedQueryExecution>(query, *this); }
