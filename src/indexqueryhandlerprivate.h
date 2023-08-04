// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "index.h"
#include <QString>
#include <memory>
#include <shared_mutex>
using namespace albert;
using namespace std;

class IndexQueryHandlerPrivate final
{
public:
    unique_ptr<Index> index;
    shared_mutex index_mutex;
    bool fuzzy;
};

