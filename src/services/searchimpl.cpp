#include "searchimpl.h"
#include <algorithm>



/**************************************************************************/
ExactMatchSearchImpl::ExactMatchSearchImpl(Index *p) : SearchImpl(p){}

/**************************************************************************/
void ExactMatchSearchImpl::query(const QString &req, QVector<Service::Item *> *res) const {
	QVector<Service::Item *>::const_iterator lb, ub;
	lb =  std::lower_bound (_parent->_index.cbegin(), _parent->_index.cend(), req, Index::CaseInsensitiveCompare());
	ub =  std::upper_bound (_parent->_index.cbegin(), _parent->_index.cend(), req, Index::CaseInsensitiveComparePrefix());
	while (lb!=ub)
		res->push_back(*(lb++));
}



/**************************************************************************/
struct WordMatchSearchImpl::CaseInsensitiveCompare
{
	inline bool operator()( Posting const &pre, Posting const &rhs ) const {return (*this)(pre.first, rhs.first);}
	inline bool operator()( QString const &pre, Posting const &rhs ) const {return (*this)(pre, rhs.first);}
	inline bool operator()( Posting const &pre, QString const &rhs ) const {return (*this)(pre.first, rhs);}
	inline bool operator()( QString const &pre, QString const &rhs ) const {return Service::CaseInsensitiveCompare()(pre, rhs);}
};
/**************************************************************************/
struct WordMatchSearchImpl::CaseInsensitiveComparePrefix
{
	inline bool operator()( Posting const &pre, Posting const &rhs ) const {return (*this)(pre.first, rhs.first);}
	inline bool operator()( QString const &pre, Posting const &rhs ) const {return (*this)(pre, rhs.first);}
	inline bool operator()( Posting const &pre, QString const &rhs ) const {return (*this)(pre.first, rhs);}
	inline bool operator()( QString const &pre, QString const &rhs ) const {return Service::CaseInsensitiveComparePrefix()(pre, rhs);}
};


#include <qdebug.h>
#include <QRegularExpression>
/**************************************************************************/
WordMatchSearchImpl::WordMatchSearchImpl(Index *p) : SearchImpl(p)
{
	// Build inverted index
	typedef QMap<QString, QSet<Service::Item*>> InvertedIndexMap;
	InvertedIndexMap invertedIndexMap;
	for (Service::Item *i : _parent->_index)
	{
		QStringList words = i->title().split(QRegExp("\\W+"), QString::SkipEmptyParts);
		for (QString &w : words)
			invertedIndexMap[w].insert(i);
	}

	// Convert back to vector for fast random access search algorithms
	for (InvertedIndexMap::const_iterator i = invertedIndexMap.cbegin(); i != invertedIndexMap.cend(); ++i)
		_invertedIndex.push_back(QPair<QString, QSet<Service::Item*>>(i.key(), i.value()));
	std::sort(_invertedIndex.begin(), _invertedIndex.end(), CaseInsensitiveCompare());

	for (InvertedIndexMap::const_iterator i = invertedIndexMap.cbegin(); i != invertedIndexMap.cend(); ++i)
		qDebug() << i.key();

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
	for (Service::Item *s : *resSet)
		res->append(s);
}


/**************************************************************************/
struct FuzzySearchImpl::CaseInsensitiveComparePrefix
{
	inline bool operator()( Posting const &pre, Posting const &rhs ) const {return (*this)(pre.first, rhs.first);}
	inline bool operator()( QString const &pre, Posting const &rhs ) const {return (*this)(pre, rhs.first);}
	inline bool operator()( Posting const &pre, QString const &rhs ) const {return (*this)(pre.first, rhs);}
	inline bool operator()( QString const &pre, QString const &rhs ) const {return Index::CaseInsensitiveComparePrefix()(pre, rhs);}
};

/**************************************************************************/
FuzzySearchImpl::FuzzySearchImpl(Index *p) : SearchImpl(p){}

/**************************************************************************/
void FuzzySearchImpl::query(const QString &req, QVector<Service::Item *> *res) const
{

}
