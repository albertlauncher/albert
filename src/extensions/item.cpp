// Copyright (c) 2023 Manuel Schneider

#include "albert/extensions/item.h"
using namespace albert;

Action::Action(QString i, QString t, std::function<void()> f):
    id(std::move(i)),
    text(std::move(t)),
    function(std::move(f)){}

QString Item::inputActionText() const { return {}; }

bool Item::hasActions() const { return true; }

std::vector<Action> Item::actions() const { return {}; }
