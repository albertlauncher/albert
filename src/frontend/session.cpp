// Copyright (c) 2024 Manuel Schneider

#include "frontend.h"
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
    q->setParent(this);  // important for qml ownership determination

    frontend_.setQuery(q.get());
    q->run();
}
