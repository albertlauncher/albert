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
#include "core_globals.h"
#include "abstractitem.h"

/**
 * @brief The Query class
 */
class EXPORT_CORE AbstractQuery
{
public:

    virtual ~AbstractQuery(){}

    /**
     * @brief Add matches
     * Adds a match to the results. Score describes a percentual match of the
     * query against the item. 0 beeing no match SHRT_MAX beeing a full match.
     * @param item The item to add
     * @param score Matchfactor of the query against the item
     */
    virtual void addMatch(SharedItem item, short score = 0) = 0;


    /**
     * @brief isValid gets the validity of the query.
     * Running handler will stop long operations and discard their matches,
     * if the query is invalid.
     * @return
     */
    virtual bool isValid() const = 0;


    /**
     * @brief Returns the search term of this query
     */
    virtual const QString & searchTerm() const = 0;

};

