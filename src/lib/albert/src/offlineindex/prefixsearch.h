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
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "indeximpl.h"

namespace Core {

class Indexable;

class PrefixSearch : public IndexImpl
{
public:

    PrefixSearch();
    PrefixSearch(const PrefixSearch &rhs);
    virtual ~PrefixSearch();

    void add(std::shared_ptr<Indexable> idxble) override;
    void clear() override;
    std::vector<std::shared_ptr<Indexable>> search(const QString &req) const override;

protected:

    typedef std::map<QString, std::set<uint>> InvertedIndex;
    InvertedIndex invertedIndex_;
    std::vector<std::shared_ptr<Indexable>> index_;
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

}
