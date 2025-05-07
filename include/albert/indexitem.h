// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/item.h>
#include <memory>

namespace albert::util
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
