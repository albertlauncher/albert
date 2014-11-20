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

#include "wordmatchsearch.h"
#include "abstractindex.h"

/**************************************************************************/
/**************************************************************************/
struct WordMatchSearch::CaseInsensitiveCompare
{
	inline bool operator()( WordMatchSearch::Posting const &pre, WordMatchSearch::Posting const &rhs ) const {
		return (*this)(pre.first, rhs.first);
	}

	inline bool operator()( QString const &pre, WordMatchSearch::Posting const &rhs ) const {
		return (*this)(pre, rhs.first);
	}

	inline bool operator()( WordMatchSearch::Posting const &pre, QString const &rhs ) const {
		return (*this)(pre.first, rhs);
	}

	inline bool operator()( QString const &lhs, QString const &rhs ) const {
		return lhs.compare(rhs, Qt::CaseInsensitive)<0;
	}
};

/**************************************************************************/
/**************************************************************************/
struct WordMatchSearch::CaseInsensitiveComparePrefix
{
	inline bool operator()( WordMatchSearch::Posting const &pre, WordMatchSearch::Posting const &rhs ) const {
		return (*this)(pre.first, rhs.first);
	}

	inline bool operator()( QString const &pre, WordMatchSearch::Posting const &rhs ) const {
		return (*this)(pre, rhs.first);
	}

	inline bool operator()( WordMatchSearch::Posting const &pre, QString const &rhs ) const {
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

/**************************************************************************/
/**************************************************************************/
void WordMatchSearch::buildIndex()
{
	_invertedIndex.clear();

	// Build inverted index
	typedef QMap<QString, QSet<Service::Item*>> InvertedIndexMap;
	InvertedIndexMap invertedIndexMap;
	for (Service::Item *i : _ref->data())
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
void WordMatchSearch::query(const QString &req, QVector<Service::Item *> *res) const
{
	QSet<Service::Item*>* resSet = nullptr;
	QStringList words = req.split(' ', QString::SkipEmptyParts);

	// Quit if there are no words in query
	if (words.empty())
		return;

	for (QString &w : words)
	{
		InvertedIndex::const_iterator lb, ub;
		lb = std::lower_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), w, CaseInsensitiveCompare());
		ub = std::upper_bound (_invertedIndex.cbegin(), _invertedIndex.cend(), w, CaseInsensitiveComparePrefix());
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
