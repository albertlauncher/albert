// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/frontend.h"
#include "queryengine.h"
using namespace albert;
using namespace std;

shared_ptr<Query> Frontend::query(const QString &query) const
{
    return query_engine->query(query);
}

void Frontend::setEngine(QueryEngine *engine)
{
    query_engine = engine;
}
