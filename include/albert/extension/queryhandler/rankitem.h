// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "item.h"
#include <memory>

namespace albert
{

///
class ALBERT_EXPORT RankItem
{
public:
    explicit RankItem(std::shared_ptr<Item> item, float score);

    /// The matched item
    std::shared_ptr<Item> item;

    /// The match score. Must be in the range (0,1]. No checks are done for performance.
    float score;
};

}
