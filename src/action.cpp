// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/query/action.h"
using namespace albert;
using namespace std;

Action::Action(QString i, QString t, ::function<void()> f):
    id(::move(i)),
    text(::move(t)),
    function(::move(f))
{}

