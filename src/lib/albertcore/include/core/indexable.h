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
#include <QString>
#include <vector>
#include "item.h"
#include "core_globals.h"

namespace Core {

/**
 * @brief The Indexable class
 * The interface items need to be indexable by the offline index
 */
class EXPORT_CORE IndexableItem : public Item
{

public:

    struct WeightedKeyword {
        WeightedKeyword(const QString& kw, uint32_t r) : keyword(kw), relevance(r){}
        QString keyword;
        uint32_t relevance;
    };

    virtual std::vector<WeightedKeyword> indexKeywords() const = 0;

};

}

