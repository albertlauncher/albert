// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include "item.h"
#include <QString>


namespace albert {

class ALBERT_EXPORT FallbackProvider : virtual public Extension
{
public:
    /// These items show up if a query yields no results
    virtual std::vector<std::shared_ptr<albert::Item>> fallbacks(const QString &) const = 0;
};

}
