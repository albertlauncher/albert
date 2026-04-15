// Copyright (c) 2023-2025 Manuel Schneider

#include "globalqueryhandler.h"
#include "usagescoring.h"
#include <QCoroGenerator>
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() {}

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery() { return {}; }

ItemGenerator GlobalQueryHandler::items(QueryContext &ctx)
{ return lazySort(ctx.usageScoring().applied(id(), rankItems(ctx))); }
