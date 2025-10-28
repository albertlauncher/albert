// Copyright (c) 2022-2025 Manuel Schneider

#include "queryhandler.h"
#include <QString>
using namespace albert;

QueryHandler::~QueryHandler() {}

QString QueryHandler::synopsis(const QString &) const { return {}; }

QString QueryHandler::defaultTrigger() const { return id() + QChar::Space; }

bool QueryHandler::allowTriggerRemap() const { return true; }

void QueryHandler::setTrigger(const QString &) {}

bool QueryHandler::supportsFuzzyMatching() const { return false; }

void QueryHandler::setFuzzyMatching(bool) { }
