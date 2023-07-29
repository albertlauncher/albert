// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/item.h"
#include <QString>
#include <memory>

namespace albert
{

/// The eligible for the internal index of IndexQueryHandler
/// @see IndexQueryHandler
class ALBERT_EXPORT IndexItem
{
public:
    /// \param item @copydoc item
    /// \param string @copydoc string
    IndexItem(std::shared_ptr<Item> item, QString string);

    /// The item to be indexed
    std::shared_ptr<Item> item;

    /// The corresponding lookup string
    QString string;
};

}
