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
#include "abstractsearch.h"
#include <QList>
#include <QString>
#include <QVector>
#include <QPair>
#include <QSet>
#include <QMap>
#include <memory>

template<typename T>
uint qHash(const std::shared_ptr<T> &p)
{
    return qHash(p.get());
}

/** ***************************************************************************/
template<class C>
class PrefixSearch final : public AbstractSearch<C>
{
    class CaseInsensitiveCompare;
    class CaseInsensitiveComparePrefix;

    typedef QPair<QString, QSet<SharedItemPtr>> Posting;
    typedef QVector<Posting> InvertedIndex;
    typedef QMap<QString, QSet<SharedItemPtr>> InvertedIndexMap;

public:
	PrefixSearch() = delete;
    explicit PrefixSearch(const C &idx, std::function<QString(SharedItemPtr)> f)
        : AbstractSearch<C>(idx, f) {}
	~PrefixSearch(){}

	/**
	 * @brief buildIndex
	 */
	void buildIndex() override
    {
        _invertedIndex.clear();

		// Build an inverted index mapping
        InvertedIndexMap invIdxMap;
        for (typename C::const_iterator it = this->_index.cbegin(); it != this->_index.cend(); ++it) {
            QStringList words = this->_textFunctor.operator()(*it).split(QRegExp(SEPARATOR), QString::SkipEmptyParts);
			for (QString &w : words)
                invIdxMap[w].insert(*it);
		}

		// Convert back to vector for fast random access search algorithms
		_invertedIndex.clear();
        for (typename InvertedIndexMap::const_iterator i = invIdxMap.cbegin(); i != invIdxMap.cend(); ++i)
			_invertedIndex.push_back(Posting(i.key(), i.value()));
		std::sort(_invertedIndex.begin(), _invertedIndex.end(), CaseInsensitiveCompare());
		_invertedIndex.squeeze();
	}

	/**
	 * @brief find
	 * @param req
	 * @param ids
	 */
    SharedItemPtrList find(const QString &req) const override
	{
        QSet<SharedItemPtr>* resSet = nullptr; // Constraint (1): resSet == nullptr
		QStringList words = req.split(SEPARATOR, QString::SkipEmptyParts);
        if (words.empty()) return SharedItemPtrList(); // Constraint (2): words is not empty
        for (QString &w : words) {
            typename InvertedIndex::const_iterator lb, ub;
			lb = std::lower_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), w, CaseInsensitiveCompare());
			ub = std::upper_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), w, CaseInsensitiveComparePrefix());
            QSet<SharedItemPtr> tmpSet;
			while (lb!=ub)
				tmpSet.unite(lb++->second);
			if (resSet == nullptr)		// (1)&&(2)  |-  Constraint resSet != nullptr (3)
                resSet = new QSet<SharedItemPtr>(tmpSet);
			else
				resSet->intersect(tmpSet);
		}
        // Safe to not check resSet != nullptr since (3) holds
        SharedItemPtrList res = resSet->toList();
        delete resSet;
        return res;
	}

private:
    InvertedIndex _invertedIndex;
};

/** ***************************************************************************/
template<class C>
struct PrefixSearch<C>::CaseInsensitiveCompare
{
    inline bool operator()( Posting const &lhs, Posting const &rhs ) const {
		return (*this)(lhs.first, rhs.first);
	}

    inline bool operator()( QString const &lhs, Posting const &rhs ) const {
		return (*this)(lhs, rhs.first);
	}

    inline bool operator()( Posting const &lhs, QString const &rhs ) const {
		return (*this)(lhs.first, rhs);
	}

	inline bool operator()( QString const &lhs, QString const &rhs ) const {
		return lhs.compare(rhs, Qt::CaseInsensitive)<0;
	}
};

/** ***************************************************************************/
template<class C>
struct PrefixSearch<C>::CaseInsensitiveComparePrefix
{
	inline bool operator()( Posting const &pre, Posting const &rhs ) const {
		return (*this)(pre.first, rhs.first);
	}

	inline bool operator()( QString const &pre, Posting const &rhs ) const {
		return (*this)(pre, rhs.first);
	}

	inline bool operator()( Posting const &pre, QString const &rhs ) const {
		return (*this)(pre.first, rhs);
	}

	inline bool operator()( QString const& pre, QString const& rhs ) const	{
		QString::const_iterator a,b;
		a = pre.cbegin();
		b = rhs.cbegin();
		QChar ca,cb;
		while (a != pre.cend() && b != rhs.cend()){
			ca = a++->toLower();
			cb = b++->toLower();
			if (ca < cb)
				return true;
			if (ca > cb)
				return false;
		}
		return false;
	}
};
