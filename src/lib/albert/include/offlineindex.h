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
#include <memory>
#include "core_globals.h"

namespace Core {

class IndexImpl;
class Indexable;

class EXPORT_CORE OfflineIndex final {

public:
    /**
     * @brief Contstructs a search
     * @param fuzzy Sets the type of the search. Defaults to false.
     */
    OfflineIndex(bool fuzzy = false);

    /**
     * @brief Destructs the search
     * @param
     */
    ~OfflineIndex();

    /**
     * @brief Sets the type of the search to fuzzy
     * @param fuzzy The type to set. Defaults to true.
     */
    void setFuzzy(bool fuzzy = true);

    /**
     * @brief Type of the search
     * @return True if the search is fuzzy else false.
     */
    bool fuzzy();

    /**
     * @brief Set the error tolerance of the fuzzy search
     *
     * If the value d is >1, the search tolerates d errors. If the value d is <1,
     * the search tolerates wordlength * d errors. The "amount of tolerance" is
     * measures in maximal prefix edit distance. If the search is not set to fuzzy
     * setDelta has no effect.
     *
     * @param t The amount of error tolerance
     */
    void setDelta(double d);

    /**
     * @brief The error tolerance of the fuzzy search
     * @return The amount of error tolerance if search is fuzzy 0 else.
     */
    double delta();

    /**
     * @brief Build the search index
     * @param The items to index
     */
    void add(std::shared_ptr<Core::Indexable> idxble);

    /**
     * @brief Clear the search index
     */
    void clear();

    /**
     * @brief Perform a search on the index
     * @param req The query string
     */
    std::vector<std::shared_ptr<Core::Indexable>> search(const QString &req) const;

private:
    IndexImpl *impl_;
};

}



