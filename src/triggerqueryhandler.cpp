// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/triggerqueryhandler.h"
using namespace albert;

QString TriggerQueryHandler::synopsis() const
{ return {}; }

QString TriggerQueryHandler::defaultTrigger() const
{ return QString("%1 ").arg(id()); }

bool TriggerQueryHandler::allowTriggerRemap() const
{ return true; }

TriggerQueryHandler::TriggerQuery::~TriggerQuery() = default;
