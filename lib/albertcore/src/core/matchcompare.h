// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
