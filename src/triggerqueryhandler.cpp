// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "triggerqueryhandlerprivate.h"
using namespace albert;

TriggerQueryHandler::TriggerQueryHandler() : d(new TriggerQueryHandlerPrivate) {}

TriggerQueryHandler::~TriggerQueryHandler() = default;

QString TriggerQueryHandler::trigger() const { return d->trigger; }

QString TriggerQueryHandler::synopsis() const { return {}; }

QString TriggerQueryHandler::defaultTrigger() const { return QString("%1 ").arg(id()); }

bool TriggerQueryHandler::allowTriggerRemap() const { return true; }

bool TriggerQueryHandler::supportsFuzzyMatching() const { return false; }

bool TriggerQueryHandler::fuzzyMatching() const { return false; }

void TriggerQueryHandler::setFuzzyMatching(bool) { }
