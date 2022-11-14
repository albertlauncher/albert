// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/queryhandler.h"
#include <utility>
using namespace albert;


Action::Action(QString id, QString text, std::function<void()> function):
    id(std::move(id)),
    text(std::move(text)),
    function(std::move(function))
{
    
}


QString Item::inputActionText() const
{
    return {}; 
}

bool Item::hasActions() const
{
    return true;
}

std::vector<Action> Item::actions() const
{ 
    return {};
}


QString QueryHandler::synopsis() const
{
    return {};
}

QString QueryHandler::default_trigger() const
{
    return QString("%1 ").arg(id());
}

bool QueryHandler::allow_trigger_remap() const
{
    return true;
}

std::vector<std::shared_ptr<Item>> QueryHandler::fallbacks(const QString &) const
{
    return {};
}
