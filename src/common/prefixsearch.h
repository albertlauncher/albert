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
#define SEPARATOR "\\W+" // TODO MAKE CONFIGURABLE
#include "abstractsearch.h"
#include <QList>
#include <QString>
#include <QVector>
#include <QPair>
#include <QSet>
#include <QHash>

class CaseInsensitiveCompare;
class CaseInsensitiveComparePrefix;

typedef QPair<QString, QSet<QString>> Posting;
typedef QVector<Posting> InvertedIndex;

template<class T>
class PrefixSearch final : public AbstractSearch<T>
{

public:
	PrefixSearch() = delete;
	explicit PrefixSearch(QHash<QString, T> *idx, std::function<QString(T)> f)
		: AbstractSearch<T>(idx, f) {}
	~PrefixSearch(){}

	/**
	 * @brief buildIndex
	 */
	void buildIndex() override
	{
		// Build an inverted index mapping
		QHash<QString, QSet<QString>> invIdxMap;
		for (typename QHash<QString, T>::iterator it = this->_index->begin(); it != this->_index->end(); ++it) {
			it.key(); it.value();
			QStringList words = this->_textFunctor.operator()(it.value()).split(QRegExp(SEPARATOR), QString::SkipEmptyParts);
			for (QString &w : words)
				invIdxMap[w].insert(it.key());
		}

		// Convert back to vector for fast random access search algorithms
		_invertedIndex.clear();
		for (QHash<QString, QSet<QString>>::const_iterator i = invIdxMap.cbegin(); i != invIdxMap.cend(); ++i)
			_invertedIndex.push_back(Posting(i.key(), i.value()));
		std::sort(_invertedIndex.begin(), _invertedIndex.end(), CaseInsensitiveCompare());
		_invertedIndex.squeeze();
	}

	/**
	 * @brief find
	 * @param req
	 * @param ids
	 */
	QStringList find(const QString &req) const override
	{
		QSet<QString>* resSet = nullptr;
		// (1): Constraint resSet == nullptr
		QStringList words = req.split(SEPARATOR, QString::SkipEmptyParts);
		if (words.empty()) return QStringList(); //TODO BLEIBT SO NICHT !MT MEMLEAK!!!!
		// (2): Constraint words  is not empty
		for (QString &w : words) {
			InvertedIndex::const_iterator lb, ub;
			lb = std::lower_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), w, CaseInsensitiveCompare());
			ub = std::upper_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), w, CaseInsensitiveComparePrefix());
			QSet<QString> tmpSet;
			while (lb!=ub)
				tmpSet.unite(lb++->second);
			if (resSet == nullptr)		// (1)&&(2)  |-  Constraint resSet != nullptr (3)
				resSet = new QSet<QString>(tmpSet);
			else
				resSet->intersect(tmpSet);
		}
		// Safe to not check resSet != nullptr since (3) holds
//		for (const QString &s : *resSet)
//			ids.to->append(s);
		return resSet->toList();
		//delete resSet; //TODO BLEIBT SO NICHT !MT MEMLEAK!!!!
	}

private:
	QVector<Posting> _invertedIndex;
};



/****************************************************************************///
struct CaseInsensitiveCompare
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

/****************************************************************************///
struct CaseInsensitiveComparePrefix
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
