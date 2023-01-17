// Copyright (c) 2023 Manuel Schneider

#include "albert/extensions/item.h"
using namespace albert;

Action::Action(QString id, QString text, std::function<void()> function):
    id(std::move(id)),
    text(std::move(text)),
    function(std::move(function)){}

QString Item::inputActionText() const { return {}; }

bool Item::hasActions() const { return true; }

std::vector<Action> Item::actions() const { return {}; }
