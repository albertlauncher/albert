// Copyright (c) 2022 Manuel Schneider

#include "albert/item.h"

albert::Action::Action(QString id, QString text, std::function<void()> function)
        : id(id), text(text), function(function) {}

albert::IndexItem::IndexItem(std::shared_ptr<Item> item, QString string)
        : item(std::move(item)), string(std::move(string)) {}

albert::RankItem::RankItem(std::shared_ptr<Item> item, albert::Score score)
        : item(std::move(item)), score(score) {}

QString albert::Item::inputActionText() const { return {}; }

bool albert::Item::hasActions() const { return true; }

std::vector<albert::Action> albert::Item::actions() const { return {}; }

