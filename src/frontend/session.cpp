// Copyright (c) 2024-2025 Manuel Schneider

#include "frontend.h"
#include "query.h"
#include "queryengine.h"
#include "queryexecution.h"
#include "session.h"
using namespace albert::detail;
using namespace albert;
using namespace std;

Session::Session(QueryEngine &e, Frontend &f) : engine_(e), frontend_(f)
{
    connect(&frontend_, &Frontend::inputChanged,
            this, &Session::runQuery);
    runQuery(frontend_.input());
}

Session::~Session()
{
    frontend_.setQuery(nullptr);
}

void Session::runQuery(const QString &query_string)
{
    if(!queries_.empty())
        queries_.back()->execution().cancel();
    auto &q = queries_.emplace_back(engine_.query(query_string));
    frontend_.setQuery(q.get());
}
