// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include <QString>

namespace albert
{
class Item;

struct Query
{
    virtual ~Query();
    virtual const QString &trigger() const = 0;  ///< The trigger of this query
    virtual const QString &string() const = 0;  ///< The query string
    virtual bool isValid() const = 0;  ///< True if query has not been cancelled
    virtual void add(const std::shared_ptr<Item> &item) = 0;  ///< Copy add item
    virtual void add(std::shared_ptr<Item> &&item) = 0;  ///< Move add item
    virtual void add(const std::vector<std::shared_ptr<Item>> &items) = 0;  ///< Copy add items
    virtual void add(std::vector<std::shared_ptr<Item>> &&items) = 0;  ///< Move add items
};

}
