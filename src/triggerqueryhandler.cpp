// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/query/triggerqueryhandler.h"
using namespace albert;

TriggerQueryHandler::~TriggerQueryHandler() = default;

QString TriggerQueryHandler::synopsis() const { return {}; }

QString TriggerQueryHandler::defaultTrigger() const { return QString("%1 ").arg(id()); }

bool TriggerQueryHandler::allowTriggerRemap() const { return true; }

void TriggerQueryHandler::setTrigger(const QString &) {}

bool TriggerQueryHandler::supportsFuzzyMatching() const { return false; }

void TriggerQueryHandler::setFuzzyMatching(bool) { }
