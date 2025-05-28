// Copyright (c) 2023-2024 Manuel Schneider

#include "item.h"
using namespace albert;

Item::~Item() = default;

QString Item::inputActionText() const { return {}; }

std::vector<Action> Item::actions() const { return {}; }

void Item::addObserver(Item::Observer*) {}

void Item::removeObserver(Item::Observer*) {}

Item::Observer::~Observer() = default;
