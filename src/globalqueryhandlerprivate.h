// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/query/globalqueryhandler.h"
#include <QString>
#include <map>
#include <shared_mutex>
#include <vector>

class GlobalQueryHandlerPrivate final
{
public:
    GlobalQueryHandlerPrivate(albert::GlobalQueryHandler *qp) : q(qp) {}
    albert::GlobalQueryHandler * const q;
};
