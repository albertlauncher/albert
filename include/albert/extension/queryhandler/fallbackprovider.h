// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include <QString>
#include <memory>
#include <vector>

namespace albert
{
class Item;

/// Fallback providing extension.
/// Use this if you want to add items to the fallbacks
/// shown if a query yielded no results
class ALBERT_EXPORT FallbackHandler : virtual public Extension
{
public:
    /// Fallbacks provided by this handler
    virtual std::vector<std::shared_ptr<Item>> fallbacks(const QString &) const = 0;
};

}



namespace albert
{
class Item;

class ALBERT_EXPORT FallbackFactory
{
public:
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual std::shared_ptr<Item> createFallbackItem(const QString &) const = 0;
};

class ALBERT_EXPORT FallbackProvider : virtual public Extension
{
public:
    /// Fallbacks provided by this handler
    virtual std::vector<std::shared_ptr<Item>> fallbacks(const QString &) const = 0;
};


}
