// Copyright (c) 2023-2025 Manuel Schneider

#include "globalqueryhandler.h"
#include "usagescoring.h"
#include <QCoroGenerator>
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() {}

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery() { return {}; }

ItemGenerator GlobalQueryHandler::items(QueryContext &ctx)
{
    auto rank_items = rankItems(ctx);
    return lazySort(::move(rank_items), ctx.usageScoring());
}
