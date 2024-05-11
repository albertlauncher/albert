// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include "albert/extension.h"
#include "item.h"
#include <QString>
#include <memory>
#include <vector>

namespace albert
{
class Item;

///
/// Abstract fallback provider.
///
/// Use this if you want to add items to the fallbacks
/// shown if a query yielded no results
///
class ALBERT_EXPORT FallbackHandler : virtual public Extension
{
public:

    /// Fallbacks provided by this handler
    virtual std::vector<std::shared_ptr<Item>> fallbacks(const QString &) const = 0;

protected:

    ~FallbackHandler() override;

};

}
