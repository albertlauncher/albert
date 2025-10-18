// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/item.h>
#include <memory>

namespace albert
{

///
/// An item utlized by ItemIndex
///
/// \ingroup query_util
///
class ALBERT_EXPORT IndexItem
{
public:
    /// Constructs an index item with the given _item_ and _string_.
    IndexItem(std::shared_ptr<Item> item, QString string);

    /// The item to be indexed
    std::shared_ptr<Item> item;

    /// The corresponding lookup string
    QString string;
};

}
