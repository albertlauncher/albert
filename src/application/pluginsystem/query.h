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
#include <QString>
#include <memory>
using std::shared_ptr;
#include <vector>

class QueryPrivate;
class AlbertItem;
class IExtension;

class Query final
{
public:
    friend class ExtensionManager;

    Query(const QString &term);
    ~Query();

    /**
     * @brief Add a top-level/root node to the albert tree
     * This does not take the ownership of the item. Remember to keep the item
     * live as long as the session is active.
     * @param node The amount of error tolerance
     */
    void addMatch(shared_ptr<AlbertItem> item, short score = 0);

    /**
     * @brief Reset the query
     * Note: Not clear what it comprises in the future, but at least clears the
     * matches
     */
    void reset();


    /**
     * @brief setValid sets the validity of the query.
     * Running handler will stop long operations and discard their matches,
     * if the query is invalid.
     * @param b
     */
    void setValid(bool b = true);


    /**
     * @brief isValid gets the validity of the query.
     * Running handler will stop long operations and discard their matsches,
     * if the query is invalid.
     * @return
     */
    bool isValid();

    /**
     * @brief Returns the search term of this query
     */
    const QString &searchTerm() const;

private:
    QueryPrivate *impl;
};

