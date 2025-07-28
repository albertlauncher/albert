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

    // important for qml ownership determination
    // CAUTION INTRODUCES NASTY BUGS
    // 1. this _was_ important and is no more required
    // 2. this leads to early query deletion since its a child of qobject
    // (earlier than intended, see above q.release()->deleteLater();)
    // q->setParent(this);

    frontend_.setQuery(q.get());
    q->run();
}
