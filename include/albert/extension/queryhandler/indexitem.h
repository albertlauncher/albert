// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "item.h"
#include <QString>
#include <memory>

namespace albert
{

///
class ALBERT_EXPORT IndexItem
{
public:
    IndexItem(std::shared_ptr<Item> item, QString string);

    std::shared_ptr<Item> item; ///< The item
    QString string; ///< The corresponding string
};

}
