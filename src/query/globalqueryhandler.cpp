// Copyright (c) 2023-2026 Manuel Schneider

#include "globalqueryhandler.h"
#include <QCoroGenerator>
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() {}

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery() { return {}; }

ItemGenerator GlobalQueryHandler::items(QueryContext ctx)
{ return lazySort(rankItems(ctx), ctx.usageScoring()); }
