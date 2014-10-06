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

#include "indexservice.h"
#include <algorithm>
#include <QMap>
#include <QSet>
#include <QString>
#include <QDebug>

/**************************************************************************/
/**************************************************************************/
/***********   P R I V A T E    I M P L E M E N T A T I O N   *************/
/**************************************************************************/
/**************************************************************************/
typedef QPair<QString, QSet<Service::Item*>> Posting;
typedef QVector<Posting> InvertedIndex;
/**************************************************************************/
struct CaseInsensitiveCompare
{
	inline bool operator()( Posting const &pre, Posting const &rhs ) const {return (*this)(pre.first, rhs.first);}
	inline bool operator()( QString const &pre, Posting const &rhs ) const {return (*this)(pre, rhs.first);}
	inline bool operator()( Posting const &pre, QString const &rhs ) const {return (*this)(pre.first, rhs);}
	inline bool operator()( QString const &pre, QString const &rhs ) const {return Service::CaseInsensitiveCompare()(pre, rhs);}
};
/**************************************************************************/
struct CaseInsensitiveComparePrefix
{
	inline bool operator()( Posting const &pre, Posting const &rhs ) const {return (*this)(pre.first, rhs.first);}
	inline bool operator()( QString const &pre, Posting const &rhs ) const {return (*this)(pre, rhs.first);}
	inline bool operator()( Posting const &pre, QString const &rhs ) const {return (*this)(pre.first, rhs);}
	inline bool operator()( QString const &pre, QString const &rhs ) const {return Service::CaseInsensitiveComparePrefix()(pre, rhs);}
};
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
class SearchImpl
{
protected:
	const QVector<Service::Item*> &_indexRef;
public:
	SearchImpl(QVector<Service::Item*> const &p) : _indexRef(p){}
	virtual ~SearchImpl(){}
	virtual void query(const QString &req, QVector<Service::Item*> *res) const = 0;
};
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
class ExactMatchSearchImpl : public SearchImpl
{
public:
	ExactMatchSearchImpl(QVector<Service::Item*> const &p);
	virtual void query(const QString &req, QVector<Service::Item*> *res) const;
};
/**************************************************************************/
ExactMatchSearchImpl::ExactMatchSearchImpl(const QVector<Service::Item *> &p) : SearchImpl(p){}
/**************************************************************************/
void ExactMatchSearchImpl::query(const QString &req, QVector<Service::Item *> *res) const {
	QVector<Service::Item *>::const_iterator lb, ub;
	lb =  std::lower_bound (_indexRef.cbegin(), _indexRef.cend(), req, Service::CaseInsensitiveCompare());
	ub =  std::upper_bound (_indexRef.cbegin(), _indexRef.cend(), req, Service::CaseInsensitiveComparePrefix());
	while (lb!=ub)
		res->push_back(*(lb++));
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
class WordMatchSearchImpl : public SearchImpl
{
	InvertedIndex _invertedIndex;
public:
	WordMatchSearchImpl(QVector<Service::Item*> const &p);
	virtual void query(const QString &req, QVector<Service::Item*> *res) const;
};
/**************************************************************************/
WordMatchSearchImpl::WordMatchSearchImpl(const QVector<Service::Item *> &p) : SearchImpl(p)
{
	// Build inverted index
	typedef QMap<QString, QSet<Service::Item*>> InvertedIndexMap;
	InvertedIndexMap invertedIndexMap;
	for (Service::Item *i : _indexRef)
	{
		QStringList words = i->title().split(QRegExp("\\W+"), QString::SkipEmptyParts);
		for (QString &w : words)
			invertedIndexMap[w].insert(i);
	}

	// Convert back to vector for fast random access search algorithms
	for (InvertedIndexMap::const_iterator i = invertedIndexMap.cbegin(); i != invertedIndexMap.cend(); ++i)
		_invertedIndex.push_back(QPair<QString, QSet<Service::Item*>>(i.key(), i.value()));
	std::sort(_invertedIndex.begin(), _invertedIndex.end(), CaseInsensitiveCompare());
	_invertedIndex.squeeze();
}
/**************************************************************************/
void WordMatchSearchImpl::query(const QString &req, QVector<Service::Item *> *res) const
{
	QSet<Service::Item*>* resSet = nullptr;
	QStringList words = req.split(' ', QString::SkipEmptyParts);
	for (QString &w : words)
	{
		InvertedIndex::const_iterator lb, ub;
		lb =  std::lower_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), w, CaseInsensitiveCompare());
		ub =  std::upper_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), w, CaseInsensitiveComparePrefix());
		QSet<Service::Item*> tmpSet;
		while (lb!=ub)
			tmpSet.unite(lb++->second);
		if (resSet == nullptr)
			resSet = new QSet<Service::Item*>(tmpSet);
		else
			resSet->intersect(tmpSet);
	}
	if (resSet != nullptr) {
		for (Service::Item *s : *resSet)
			res->append(s);
		delete resSet;
	}
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
class FuzzySearchImpl : public SearchImpl
{
	typedef QMap<QString, QSet<Service::Item *>> qGramIndex;
	qGramIndex _qGramIndex;
	static const int q;
public:
	FuzzySearchImpl(QVector<Service::Item*> const &p);
	virtual void query(const QString &req, QVector<Service::Item*> *res) const;
};
/**************************************************************************/
const int FuzzySearchImpl::q = 3;
/**************************************************************************/
FuzzySearchImpl::FuzzySearchImpl(const QVector<Service::Item *> &p) : SearchImpl(p)
{
	// Build qGramIndex
	for (Service::Item *item : _indexRef) {
		//Split the name into words
		QStringList words = item->title().split(QRegExp("\\W+"), QString::SkipEmptyParts);
		for (QString &w : words){
			//Split the word into qGrams
			QString spaced = QString("  ").append(w).append("  ");
			for (int i = 0 ; i < w.size()+q-1; ++i)
				// Save a reference to this enty under the key qGram
				_qGramIndex[spaced.mid(i,q)].insert(item);
		}
	}
}
/**************************************************************************/
void FuzzySearchImpl::query(const QString &req, QVector<Service::Item *> *res) const
{
	// Extract the qGrams of the query
	QSet<QString> qGrams;
	// Split the query into words
	QStringList words = req.split(QRegExp("\\W+"), QString::SkipEmptyParts);
	for (QString &w : words){
		// Split the word into qGrams
		QString spaced = QString("  ").append(w).append("  ");
		for (int i = 0 ; i < w.size()+q-1; ++i)
			// Save the qgram
			qGrams.insert(spaced.mid(i,q));
	}

	// Get the intersection of the entries referenced by the qgrams
	QSet<Service::Item*>* resSet = nullptr;
	for(QString qGram : qGrams){
		if (resSet == nullptr)
			resSet = new QSet<Service::Item*>(_qGramIndex[qGram]);
		else
			resSet->intersect(_qGramIndex[qGram]);
	}

	// Convert to vector
	if (resSet != nullptr) {
		for (Service::Item *s : *resSet)
			res->append(s);
		delete resSet;
	}

}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/





































/**************************************************************************/
IndexService::IndexService()
{
	_search =  new ExactMatchSearchImpl(_index);
	_searchType = IndexService::SearchType::Exact;
}

/**************************************************************************/
IndexService::~IndexService()
{
	delete _search;
}

/**************************************************************************/
inline void IndexService::query(const QString&req, QVector<Service::Item*>*res) const noexcept {
	_search->query(req, res);
}

/**************************************************************************/
void IndexService::setSearchType(IndexService::SearchType T)
{
	_searchType = T;
	switch (T) {
	case IndexService::SearchType::Exact:
		if (!dynamic_cast<ExactMatchSearchImpl*>(_search)){
			delete _search;
			_search = new ExactMatchSearchImpl(_index);
			qDebug() << "Set searchtype to ExactMatch";
		}
		break;
	case IndexService::SearchType::WordMatch:
		if (!dynamic_cast<WordMatchSearchImpl*>(_search)){
			delete _search;
			_search = new WordMatchSearchImpl(_index);
			qDebug() << "Set searchtype to WordMatch";
		}
		break;
	case IndexService::SearchType::Fuzzy:
		if (!dynamic_cast<FuzzySearchImpl*>(_search)){
			delete _search;
			_search = new FuzzySearchImpl(_index);
			qDebug() << "Set searchtype to Fuzzy";
		}
		break;
	}
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
