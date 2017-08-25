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


#include "offlineindex.h"
#include "indexable.h"
#include "prefixsearch.h"
#include "fuzzysearch.h"


/** ***************************************************************************/
Core::OfflineIndex::OfflineIndex(bool fuzzy) {
    (fuzzy) ? impl_ = new FuzzySearch() : impl_ = new PrefixSearch();
}



/** ***************************************************************************/
Core::OfflineIndex::~OfflineIndex() {
    delete impl_;
}



/** ***************************************************************************/
void Core::OfflineIndex::setFuzzy(bool fuzzy) {
    if (dynamic_cast<FuzzySearch*>(impl_)) {
        if (fuzzy) return;
        FuzzySearch *old = dynamic_cast<FuzzySearch*>(impl_);
        impl_ = new PrefixSearch(*old);
        delete old;
    } else if (dynamic_cast<PrefixSearch*>(impl_)) {
        if (!fuzzy) return;
        PrefixSearch *old = dynamic_cast<PrefixSearch*>(impl_);
        impl_ = new FuzzySearch(*old);
        delete old;
    } else {
        throw; //should not happen
    }
}



/** ***************************************************************************/
bool Core::OfflineIndex::fuzzy() {
    return dynamic_cast<FuzzySearch*>(impl_) != nullptr;
}



/** ***************************************************************************/
void Core::OfflineIndex::setDelta(double d) {
    FuzzySearch* f = dynamic_cast<FuzzySearch*>(impl_);
    if (f)
        f->setDelta(d);
}



/** ***************************************************************************/
double Core::OfflineIndex::delta() {
    FuzzySearch* f = dynamic_cast<FuzzySearch*>(impl_);
    if (f)
        return f->delta();
    return 0;
}



/** ***************************************************************************/
void Core::OfflineIndex::add(const std::shared_ptr<Core::IndexableItem> &idxble) {
    impl_->add(idxble);
}



/** ***************************************************************************/
void Core::OfflineIndex::clear() {
    impl_->clear();
}



/** ***************************************************************************/
std::vector<std::shared_ptr<Core::IndexableItem> > Core::OfflineIndex::search(const QString &req) const {
    return impl_->search(req);
}
