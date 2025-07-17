// Copyright (c) 2023-2025 Manuel Schneider

#include "app.h"
#include "queryengine.h"
#include "triggerqueryhandler.h"
using namespace albert;

TriggerQueryHandler::~TriggerQueryHandler() = default;

QString TriggerQueryHandler::synopsis(const QString &) const { return {}; }

QString TriggerQueryHandler::defaultTrigger() const { return QString("%1 ").arg(id()); }

bool TriggerQueryHandler::allowTriggerRemap() const { return true; }

void TriggerQueryHandler::setTrigger(const QString &) {}

bool TriggerQueryHandler::supportsFuzzyMatching() const { return false; }

void TriggerQueryHandler::setFuzzyMatching(bool) { }

void TriggerQueryHandler::applyUsageScore(std::vector<RankItem> &rank_items)
{ App::instance()->queryEngine().usageScoring().modifyMatchScores(*this, rank_items); }

