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


#include "search.h"
#include "search_impl.h"
#include "iindexable.h"
#include "prefixsearch.h"
#include "fuzzysearch.h"


/** ***************************************************************************/
Search::Search(bool fuzzy) {
    (fuzzy) ? _impl = new FuzzySearch() : _impl = new PrefixSearch();
}



/** ***************************************************************************/
Search::~Search() {
    delete _impl;
}



/** ***************************************************************************/
void Search::setFuzzy(bool fuzzy) {
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



/** ***************************************************************************/
bool Search::fuzzy() {
    return dynamic_cast<FuzzySearch*>(_impl) != nullptr;
}



/** ***************************************************************************/
void Search::setDelta(double d) {
    FuzzySearch* f = dynamic_cast<FuzzySearch*>(_impl);
    if (f)
        f->setDelta(d);
}



/** ***************************************************************************/
double Search::delta() {
    FuzzySearch* f = dynamic_cast<FuzzySearch*>(_impl);
    if (f)
        return f->delta();
    return 0;
}



/** ***************************************************************************/
void Search::add(std::shared_ptr<IIndexable> idxble) {
    _impl->add(idxble);
}



/** ***************************************************************************/
void Search::clear() {
    _impl->clear();
}



/** ***************************************************************************/
std::vector<std::shared_ptr<IIndexable> > Search::search(const QString &req) const {
    return _impl->search(req);
}
