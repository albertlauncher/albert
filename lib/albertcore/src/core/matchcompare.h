// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <memory>

namespace Core {

class Item;

/**
 * @brief The MatchOrder class
 * The implements the order of the results
 */
class MatchCompare
{
public:
    bool operator()(const std::pair<std::shared_ptr<Item>, uint>& lhs,
                    const std::pair<std::shared_ptr<Item>, uint>& rhs);
};

}
