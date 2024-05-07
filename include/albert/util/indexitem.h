// Copyright (c) 2021-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include "albert/query/item.h"
#include <QString>
#include <memory>

namespace albert
{

/// An item utlized by ItemIndex
class ALBERT_EXPORT IndexItem
{
public:
    /// \param item \copydoc item
    /// \param string \copydoc string
    IndexItem(std::shared_ptr<Item> item, QString string);

    /// The item to be indexed
    std::shared_ptr<Item> item;

    /// The corresponding lookup string
    QString string;
};

}
