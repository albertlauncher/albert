// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <vector>
#include "standarditem.h"
#include "core_globals.h"
#include "indexable.h"

namespace Core {

/** ****************************************************************************
* @brief A standard index item
* If you dont need the flexibility subclassing the abstract classes provided,
* you can simply use this container, fill it with data.
*/
class EXPORT_CORE StandardIndexItem final : public StandardItem, public Indexable
{
public:

    StandardIndexItem(const QString &id);

    virtual std::vector<Core::Indexable::WeightedKeyword> indexKeywords() const override;
    virtual void setIndexKeywords(std::vector<Indexable::WeightedKeyword> &&indexKeywords);

private:

    std::vector<Indexable::WeightedKeyword> indexKeywords_;

};

}
