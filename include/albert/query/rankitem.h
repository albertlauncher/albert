// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include "item.h"
#include <memory>

namespace albert
{

///
/// Scored item
/// Used to rank item results of mutliple handlers
///
class ALBERT_EXPORT RankItem
{
public:
    /// \param item @copydoc item
    /// \param score @copydoc score
    explicit RankItem(std::shared_ptr<Item> item, double score);

    /// The matched item
    std::shared_ptr<Item> item;

    /// The match score. Must be in the range (0,1]. Not checked for performance.
    double score;
};

}
