// Copyright (c) 2023-2025 Manuel Schneider

#include "globalqueryhandler.h"
#include "usagescoring.h"
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() {}

vector<RankItem> GlobalQueryHandler::rankItems(Query &query) { return handleGlobalQuery(query); }

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery() { return {}; }
