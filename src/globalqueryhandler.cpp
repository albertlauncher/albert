// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "globalqueryhandlerprivate.h"
using namespace albert;

GlobalQueryHandler::GlobalQueryHandler() : d(new GlobalQueryHandlerPrivate(this)) {}

GlobalQueryHandler::~GlobalQueryHandler() = default;

GlobalQueryHandler::GlobalQuery::~GlobalQuery() = default;
