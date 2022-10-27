// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include "item.h"
#include <QString>


namespace albert {

struct ALBERT_EXPORT FallbackProvider : virtual public Extension
{
    /// These items show up if a query yields no results
    virtual std::vector<SharedItem> fallbacks(const QString &) const = 0;
};

}
