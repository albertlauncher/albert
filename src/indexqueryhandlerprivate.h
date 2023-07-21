// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "index.h"
#include <QString>
#include <memory>

class IndexQueryHandlerPrivate final
{
public:
    std::unique_ptr<Index> index;
    bool fuzzy;
};

