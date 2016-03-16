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
#include <QRegularExpression>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <memory>
using std::vector;
using std::set;
using std::map;
using std::shared_ptr;
using std::unique_ptr;
#include "search_impl.h"

class PrefixSearch : public SearchImpl
{
public:
    /** ***********************************************************************/
    PrefixSearch(){}



    /** ***********************************************************************/
    PrefixSearch(const PrefixSearch &rhs) {
        invertedIndex_ = rhs.invertedIndex_;
    }



    /** ***********************************************************************/
    virtual ~PrefixSearch(){}



    /** ***********************************************************************/
    void add(shared_ptr<IIndexable> idxble) override {
        vector<QString> aliases = idxble->aliases();
        for (const QString &str : aliases) {
            // Build an inverted index
            QStringList words = str.split(QRegularExpression(SEPARATOR_REGEX), QString::SkipEmptyParts);
            for (QString &w : words) {
                invertedIndex_[w.toLower()].insert(idxble);
            }
        }
    }



    /** ***********************************************************************/
    void clear() override {
        invertedIndex_.clear();
    }



    /** ***********************************************************************/
    vector<shared_ptr<IIndexable>> search(const QString &req) const override {


        // Split the query into words W
        QStringList words = req.split(QRegularExpression(SEPARATOR_REGEX), QString::SkipEmptyParts);

        // Skip if there arent any // CONSTRAINT (2): |W| > 0
        if (words.empty())
            return vector<shared_ptr<IIndexable>>();

        set<shared_ptr<IIndexable>> resultsSet;
        QStringList::iterator wordIterator = words.begin();

        // Make lower for case insensitivity
        QString word = wordIterator++->toLower();

        // Get a word mapping once before goint to handle intersections
        for (InvertedIndex::const_iterator lb = invertedIndex_.lower_bound(word);
             lb != invertedIndex_.cend() && lb->first.startsWith(word); ++lb)
            resultsSet.insert(lb->second.begin(), lb->second.end());


        for (;wordIterator != words.end(); ++wordIterator) {

            // Make lower for case insensitivity
            word = wordIterator->toLower();

            // Unite the sets that are mapped by words that begin with word
            // w ∈ W. This set is called U_w
            set<shared_ptr<IIndexable>> wordMappingsUnion;
            for (InvertedIndex::const_iterator lb = invertedIndex_.lower_bound(word);
                 lb != invertedIndex_.cend() && lb->first.startsWith(word); ++lb)
                wordMappingsUnion.insert(lb->second.begin(), lb->second.end());


            // Intersect all sets U_w with the results
            set<shared_ptr<IIndexable>> intersection;
            std::set_intersection(resultsSet.begin(), resultsSet.end(),
                                  wordMappingsUnion.begin(), wordMappingsUnion.end(),
                                  std::inserter(intersection, intersection.begin()));
            resultsSet = std::move(intersection);
        }

        // Convert to a std::vector
        vector<shared_ptr<IIndexable>> resultsVector(resultsSet.begin(), resultsSet.end());
        return resultsVector;
    }

protected:
    typedef map<QString, set<shared_ptr<IIndexable>>> InvertedIndex;
    InvertedIndex invertedIndex_;
};
























//    class CaseInsensitiveCompare;
//    class CaseInsensitiveComparePrefix;

//template<class C>
//struct PrefixSearch<C>::CaseInsensitiveCompare
//{
//    inline bool operator()( Posting const &lhs, Posting const &rhs ) const {
//		return (*this)(lhs.first, rhs.first);
//	}

//    inline bool operator()( QString const &lhs, Posting const &rhs ) const {
//		return (*this)(lhs, rhs.first);
//	}

//    inline bool operator()( Posting const &lhs, QString const &rhs ) const {
//		return (*this)(lhs.first, rhs);
//	}

//	inline bool operator()( QString const &lhs, QString const &rhs ) const {
//		return lhs.compare(rhs, Qt::CaseInsensitive)<0;
//	}
//};

///** ***************************************************************************/
//template<class C>
//struct PrefixSearch<C>::CaseInsensitiveComparePrefix
//{
//	inline bool operator()( Posting const &pre, Posting const &rhs ) const {
//		return (*this)(pre.first, rhs.first);
//	}

//	inline bool operator()( QString const &pre, Posting const &rhs ) const {
//		return (*this)(pre, rhs.first);
//	}

//	inline bool operator()( Posting const &pre, QString const &rhs ) const {
//		return (*this)(pre.first, rhs);
//	}

//	inline bool operator()( QString const& pre, QString const& rhs ) const	{
//		QString::const_iterator a,b;
//		a = pre.cbegin();
//		b = rhs.cbegin();
//		QChar ca,cb;
//		while (a != pre.cend() && b != rhs.cend()) {
//			ca = a++->toLower();
//			cb = b++->toLower();
//			if (ca < cb)
//				return true;
//			if (ca > cb)
//				return false;
//		}
//		return false;
//	}
//};
