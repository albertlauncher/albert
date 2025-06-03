// Copyright (c) 2023-2024 Manuel Schneider

#include "item.h"
#include <set>
using namespace albert;

Item::~Item() = default;

QString Item::inputActionText() const { return {}; }

std::vector<Action> Item::actions() const { return {}; }

void Item::addObserver(Item::Observer*) {}

void Item::removeObserver(Item::Observer*) {}


Item::Observer::~Observer() = default;


class detail::DynamicItem::Private
{
public:
    std::set<Item::Observer*> observers;
};

detail::DynamicItem::DynamicItem() :
    d(std::make_unique<detail::DynamicItem::Private>())
{}

detail::DynamicItem::~DynamicItem() {}

void detail::DynamicItem::dataChanged() const
{
    for (auto observer : d->observers)
        observer->notify(this);
}

void detail::DynamicItem::addObserver(Item::Observer *o) { d->observers.insert(o); }

void detail::DynamicItem::removeObserver(Item::Observer *o) { d->observers.erase(o); }

