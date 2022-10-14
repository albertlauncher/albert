// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QString>
#include <vector>
#include <memory>
#include "export.h"

namespace Core {

class Item;

class ALBERT_EXPORT FallbackProvider
{
public:

    virtual ~FallbackProvider() {}

    /**
     * @brief Fallbacks
     * This items show up if a query yields no results
     */
    virtual std::vector<std::shared_ptr<Item>> fallbacks(const QString &) = 0;

};

}
