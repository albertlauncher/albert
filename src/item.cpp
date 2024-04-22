// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/item.h"
using namespace albert;

QString Item::inputActionText() const
{ return {}; }

std::vector<Action> Item::actions() const
{ return {}; }

bool Item::hasActions() const
{ return true; }

bool Item::dragEnabled() const
{ return true; }

bool Item::dropEnabled() const
{ return true; }
