// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#include "index.h"
#include <algorithm>
#include <QMap>
#include <QSet>
#include <QString>

/**************************************************************************/
class Index::SearchImpl
{
protected:
	Index *_parent;
public:
	SearchImpl(Index *p) : _parent(p){}
	virtual ~SearchImpl(){}
	virtual void query(const QString &req, QVector<Service::Item*> *res) const = 0;
};

/**************************************************************************/
class ExactMatchSearchImpl : public Index::SearchImpl
{
public:
	ExactMatchSearchImpl(Index *p) : SearchImpl(p){}
	virtual void query(const QString &req, QVector<Service::Item*> *res) const {
		QVector<Service::Item *>::const_iterator lb, ub;
		lb =  std::lower_bound (_parent->_index.cbegin(), _parent->_index.cend(), req, Index::CaseInsensitiveComparePrefix());
		ub =  std::upper_bound (_parent->_index.cbegin(), _parent->_index.cend(), req, Index::CaseInsensitiveComparePrefix());
		while (lb!=ub)
			res->push_back(*(lb++));
	}
};

/**************************************************************************/
class WordMatchSearchImpl : public Index::SearchImpl
{

	typedef QPair<QString, QSet<Service::Item*>> Posting;
	typedef QVector<Posting> InvertedIndex;

	struct CaseInsensitiveComparePrefix
	{
		inline bool operator()( Posting const &pre, Posting const &rhs ) const {return (*this)(pre.first, rhs.first);}
		inline bool operator()( QString const &pre, Posting const &rhs ) const {return (*this)(pre, rhs.first);}
		inline bool operator()( Posting const &pre, QString const &rhs ) const {return (*this)(pre.first, rhs);}
		inline bool operator()( QString const &pre, QString const &rhs ) const {return Index::CaseInsensitiveComparePrefix()(pre, rhs);}
	};

	InvertedIndex _invertedIndex;

public:
	WordMatchSearchImpl(Index *p) : SearchImpl(p)
	{
		// Build inverted index
		typedef QMap<QString, QSet<Service::Item*>> InvertedIndexMap;
		InvertedIndexMap invertedIndexMap;
		for (Index::Item *i : _parent->_index)
		{
			QStringList words = i->title().split(' ', QString::SkipEmptyParts);
			for (QString &w : words)
				invertedIndexMap[w].insert(i);
		}

		// Convert back to vector for fast random access search algorithms
		for (InvertedIndexMap::const_iterator i = invertedIndexMap.cbegin(); i != invertedIndexMap.cend(); ++i)
			_invertedIndex.push_back(QPair<QString, QSet<Service::Item*>>(i.key(), i.value()));
		_invertedIndex.squeeze();
	}

	virtual void query(const QString &req, QVector<Service::Item*> *res) const
	{
		QSet<Service::Item*>* resSet = nullptr;
		QStringList words = req.split(' ', QString::SkipEmptyParts);
		for (QString &w : words)
		{
			InvertedIndex::const_iterator lb, ub;
			lb =  std::lower_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), req, CaseInsensitiveComparePrefix());
			ub =  std::upper_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), req, CaseInsensitiveComparePrefix());
			QSet<Service::Item*> tmpSet;
			while (lb!=ub)
				tmpSet.unite(lb++->second);
			if (resSet == nullptr)
				resSet = new QSet<Service::Item*>(tmpSet);
			else
				resSet->intersect(tmpSet);
		}
		for (Service::Item *s : *resSet)
			res->append(s);
	}
};

/**************************************************************************/
class FuzzySearchImpl : public Index::SearchImpl
{
//	QMap<QString, QSet<Service::Item*>> _invertedIndex;
public:
	FuzzySearchImpl(Index *p) : SearchImpl(p){}
	virtual void query(const QString &req, QVector<Service::Item*> *res) const
	{

	}
};

/**************************************************************************/
Index::Index()
{
	_search =  new ExactMatchSearchImpl(this);
}

/**************************************************************************/
Index::~Index()
{
	delete _search;
}

/**************************************************************************/
inline void Index::query(const QString&req, QVector<Service::Item*>*res) const noexcept {
	_search->query(req, res);
}

/**************************************************************************/
void Index::setSearchType(Index::SearchType T)
{
	switch (T) {
	case Index::SearchType::Exact:
		if (!dynamic_cast<ExactMatchSearchImpl*>(_search)){
			delete _search;
			_search = new ExactMatchSearchImpl(this);
		}
		break;
	case Index::SearchType::WordMatch:
		if (!dynamic_cast<WordMatchSearchImpl*>(_search)){
			delete _search;
			_search = new WordMatchSearchImpl(this);
		}
		break;
	case Index::SearchType::Fuzzy:
		if (!dynamic_cast<FuzzySearchImpl*>(_search)){
			delete _search;
			_search = new FuzzySearchImpl(this);
		}
		break;
	}
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
