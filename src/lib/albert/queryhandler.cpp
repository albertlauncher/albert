// Copyright (c) 2022 Manuel Schneider

#include "albert/queryhandler.h"


QString albert::QueryHandler::synopsis() const
{
    return {};
}

QString albert::QueryHandler::default_trigger() const
{
    return id();
}

bool albert::QueryHandler::allow_trigger_remap() const
{
    return true;
}
