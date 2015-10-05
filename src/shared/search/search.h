// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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
#include "search_impl.h"
#include "iindexable.h"
#include "prefixsearch.h"
#include "fuzzysearch.h"
#include <vector>
#include <memory>

class Search final {

public:

    /**
     * @brief Contstructs a search
     * @param fuzzy Sets the type of the search. Defaults to false.
     */
    Search(bool fuzzy = false) {
        (fuzzy) ? _impl = new FuzzySearch() : _impl = new PrefixSearch();
    }



    /**
     * @brief Destructs the search
     * @param
     */
    ~Search() {
        delete _impl;
    }



    /**
     * @brief Sets the type of the search to fuzzy
     * @param fuzzy The type to set. Defaults to true.
     */
    void setFuzzy(bool fuzzy = true) {
        if (dynamic_cast<FuzzySearch*>(_impl)) {
            if (fuzzy) return;
            FuzzySearch *old = dynamic_cast<FuzzySearch*>(_impl);
            _impl = new PrefixSearch(*old);
            delete old;
        } else if (dynamic_cast<PrefixSearch*>(_impl)) {
            if (!fuzzy) return;
            PrefixSearch *old = dynamic_cast<PrefixSearch*>(_impl);
            _impl = new FuzzySearch(*old);
            delete old;
        } else {
            throw; //should not happen
        }
    }



    /**
     * @brief Type of the search
     * @return True if the search is fuzzy else false.
     */
    bool fuzzy() {
        return dynamic_cast<FuzzySearch*>(_impl) != nullptr;
    }



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
    void setDelta(double d) {
        FuzzySearch* f = dynamic_cast<FuzzySearch*>(_impl);
        if (f)
            f->setDelta(d);
    }



    /**
     * @brief The error tolerance of the fuzzy search
     * @return The amount of error tolerance if search is fuzzy 0 else.
     */
    double delta() {
        FuzzySearch* f = dynamic_cast<FuzzySearch*>(_impl);
        if (f)
            return f->delta();
        return 0;
    }



    /**
     * @brief Build the search index
     * @param The items to index
     */
    inline void add(std::shared_ptr<IIndexable> idxble) {
        _impl->add(idxble);
    }



    /**
     * @brief Clear the search index
     */
    inline void clear() {
        _impl->clear();
    }



    /**
     * @brief Perform a search on the index
     * @param req The query string
     */
    inline std::vector<std::shared_ptr<IIndexable>> search(const QString &req) const {
        return _impl->search(req);
    }

private:
    SearchImpl *_impl;
};



