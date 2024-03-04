// Copyright (c) 2024 Manuel Schneider

#include "albert/frontend/frontend.h"
#include "query.h"
#include "albert/logging.h"
#include "queryengine.h"
#include "session.h"
using namespace albert;
using namespace std;

Session::Session(QueryEngine &e, albert::Frontend &f) : engine_(e), frontend_(f)
{
    connect(&frontend_, &Frontend::inputChanged,
            this, &Session::runQuery);
    runQuery(frontend_.input());
}

Session::~Session()
{
    disconnect(&frontend_, &Frontend::inputChanged,
               this, &Session::runQuery);
    frontend_.setQuery(nullptr);
    if(!queries_.empty())
        queries_.back()->cancel();
    for (auto &q : queries_)
        q.release()->deleteLater();
}

void Session::runQuery(const QString &query_string)
{
    if(!queries_.empty())
        queries_.back()->cancel();
    auto &q = queries_.emplace_back(engine_.query(query_string));
    frontend_.setQuery(q.get());
    q->run();
}
