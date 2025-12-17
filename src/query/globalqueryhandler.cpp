// Copyright (c) 2023-2025 Manuel Schneider

#include "globalqueryhandler.h"
using namespace albert;
using namespace std;

GlobalQueryHandler::~GlobalQueryHandler() {}

vector<shared_ptr<Item>> GlobalQueryHandler::handleEmptyQuery() { return {}; }
