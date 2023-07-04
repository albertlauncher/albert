// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/action.h"

albert::Action::Action(QString i, QString t, std::function<void()> f):
    id(std::move(i)), text(std::move(t)), function(std::move(f))
{}

