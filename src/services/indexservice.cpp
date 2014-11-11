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
#include <QVector>

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
	inline bool operator()( QString const &pre, QString const &rhs ) const {return Service::Item::CaseInsensitiveCompare()(pre, rhs);}
};
/**************************************************************************/
struct CaseInsensitiveComparePrefix
{
	inline bool operator()( Posting const &pre, Posting const &rhs ) const {return (*this)(pre.first, rhs.first);}
	inline bool operator()( QString const &pre, Posting const &rhs ) const {return (*this)(pre, rhs.first);}
	inline bool operator()( Posting const &pre, QString const &rhs ) const {return (*this)(pre.first, rhs);}
	inline bool operator()( QString const &pre, QString const &rhs ) const {return Service::Item::CaseInsensitiveComparePrefix()(pre, rhs);}
};
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
class SearchImpl
{
protected:
	QVector<Service::Item*> &_indexRef;
public:
	SearchImpl(QVector<Service::Item*> &p) : _indexRef(p){}
	virtual ~SearchImpl(){}
	virtual void prepare() = 0;
	virtual void query(const QString &req, QVector<Service::Item*> *res) const = 0;
};
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
class ExactMatchSearchImpl : public SearchImpl
{
public:
	ExactMatchSearchImpl(QVector<Service::Item*> &p);
	virtual void prepare();
	virtual void query(const QString &req, QVector<Service::Item*> *res) const;
};
/**************************************************************************/
ExactMatchSearchImpl::ExactMatchSearchImpl(QVector<Service::Item *> &p) : SearchImpl(p)
{
	prepare();
}
/**************************************************************************/
void ExactMatchSearchImpl::prepare()
{
	std::sort(_indexRef.begin(), _indexRef.end(), Service::Item::CaseInsensitiveCompare());
}
/**************************************************************************/
void ExactMatchSearchImpl::query(const QString &req, QVector<Service::Item *> *res) const
{
	QVector<Service::Item *>::const_iterator lb, ub;
	lb =  std::lower_bound (_indexRef.cbegin(), _indexRef.cend(), req, Service::Item::CaseInsensitiveCompare());
	ub =  std::upper_bound (_indexRef.cbegin(), _indexRef.cend(), req, Service::Item::CaseInsensitiveComparePrefix());
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
	WordMatchSearchImpl(QVector<Service::Item *> &p);
	virtual void prepare();
	virtual void query(const QString &req, QVector<Service::Item*> *res) const;
};
/**************************************************************************/
WordMatchSearchImpl::WordMatchSearchImpl(QVector<Service::Item *> &p) : SearchImpl(p)
{
	prepare();
}
/**************************************************************************/
void WordMatchSearchImpl::prepare()
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

	// Quit if there are no words in query
	if (words.empty())
		return;

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
	// Map of words, containing their item references
	typedef QMap<QString, QSet<Service::Item*>> InvertedIndex;
	InvertedIndex _invertedIndex;

	// Map of qGrams, containing their word references and #occurences
	typedef QMap<QString, QMap<QString, unsigned int>> QGramIndex;
	QGramIndex _qGramIndex;

	// Length of the grams
	unsigned int _q;
	inline unsigned int q(){return _q;}
	inline void setQ(unsigned int q){_q=q; buildIndex();}

	// Max allowed errors
	unsigned int _delta;
	inline unsigned int delta(){return _delta;}
	inline void setDelta(unsigned int d){_delta=d;}

	void buildIndex();
	bool checkPrefixEditDistance(const QString& prefix, const QString& str, unsigned int delta) const;

public:
	FuzzySearchImpl(QVector<Service::Item*> &p, unsigned int q = 3, unsigned int delta = 2);
	virtual void prepare();
	virtual void query(const QString &req, QVector<Service::Item*> *res) const;
};
/**************************************************************************/
FuzzySearchImpl::FuzzySearchImpl(QVector<Service::Item *> &p, unsigned int q, unsigned int delta)
	: SearchImpl(p), _q(q), _delta(delta)
{
	prepare();
}
/**************************************************************************/
void FuzzySearchImpl::prepare()
{
	buildIndex();
}
/**************************************************************************/
void FuzzySearchImpl::buildIndex()
{
	_invertedIndex.clear();
	_qGramIndex.clear();

	// Build inverted index
	for (Service::Item *item : _indexRef) {
		QStringList words = item->title().split(QRegExp("\\W+"), QString::SkipEmptyParts);
		for (QString &w : words)
			_invertedIndex[w.toLower()].insert(item);
	}

	// Build qGramIndex
	for (InvertedIndex::const_iterator it = _invertedIndex.cbegin(); it != _invertedIndex.cend(); ++it)
	{
		//Split the word into lowercase qGrams
		QString spaced = QString(_q-1,' ').append(it.key().toLower());
		for (unsigned int i = 0 ; i < static_cast<unsigned int>(it.key().size()); ++i)
			// Increment #occurences of this qGram in this word
			++_qGramIndex[spaced.mid(i,_q)][it.key()];
	}
}
/**************************************************************************/
bool FuzzySearchImpl::checkPrefixEditDistance(const QString& prefix, const QString& str, unsigned int delta) const
{
  unsigned int n = prefix.size() + 1;
  unsigned int m = std::min(prefix.size() + delta + 1, static_cast<unsigned int>(str.size()) + 1);
  unsigned int matrix[n][m];

  // Initialize left and top row.
  for (unsigned int i = 0; i < n; ++i) { matrix[i][0] = i; }
  for (unsigned int i = 0; i < m; ++i) { matrix[0][i] = i; }

  // Now fill the whole matrix.
  for (unsigned int i = 1; i < n; ++i) {
	for (unsigned int j = 1; j < m; ++j) {
	  unsigned int dia = matrix[i - 1][j - 1] + (prefix[i - 1] == str[j - 1] ? 0 : 1);
	  matrix[i][j] = std::min(std::min(
		  dia,
		  matrix[i][j - 1] + 1),
		  matrix[i - 1][j] + 1);
	}
  }
  // Check the last row if there is an entry <= delta.
  for (unsigned int j = 0; j < m; ++j) {
	if (matrix[n - 1][j] <= delta) {
//		qDebug() << prefix << "~" << str << matrix[n - 1][j];
	  return true;
	}
  }
  return false;
}
/**************************************************************************/
void FuzzySearchImpl::query(const QString &req, QVector<Service::Item *> *res) const
{
	QVector<QString> words;
	for (QString &word : req.split(QRegExp("\\W+"), QString::SkipEmptyParts))
		words.append(word.toLower());
	QVector<QMap<Service::Item *, unsigned int>> resultsPerWord;

	// Quit if there are no words in query
	if (words.empty())
		return;

	// Split the query into words
	for (QString &word : words)
	{
		unsigned int delta = word.size()/3;

		// Get qGrams with counts of this word
		QMap<QString, unsigned int> qGrams;
		QString spaced(_q-1,' ');
		spaced.append(word.toLower());
		for (unsigned int i = 0 ; i < static_cast<unsigned int>(word.size()); ++i)
			++qGrams[spaced.mid(i,_q)];

		// Get the words referenced by each qGram an increment their
		// reference counter
		QMap<QString, unsigned int> wordMatches;
		// Iterate over the set of qgrams in the word
		for (QMap<QString, unsigned int>::const_iterator it = qGrams.cbegin(); it != qGrams.end(); ++it)
		{
			// Iterate over the set of words referenced by this qGram
			for (QMap<QString, unsigned int>::const_iterator wit = _qGramIndex[it.key()].begin(); wit != _qGramIndex[it.key()].cend(); ++wit)
			{
				// CRUCIAL: The match can contain only the commom amount of qGrams
				wordMatches[wit.key()] += (it.value() < wit.value()) ? it.value() : wit.value();
			}
		}

		// Allocate a new set
		resultsPerWord.push_back(QMap<Service::Item *, unsigned int>());
		QMap<Service::Item *, unsigned int>& resultsRef = resultsPerWord.back();

		// Unite the items referenced by the words accumulating their #matches
		for (QMap<QString, unsigned int>::const_iterator wm = wordMatches.begin(); wm != wordMatches.cend(); ++wm)
		{
//			// Do some kind of (cheap) preselection by mathematical bound
//			if (wm.value() < qGrams.size()-delta*_q)
//				continue;

			// Now check the (expensive) prefix edit distance
			if (!checkPrefixEditDistance(word, wm.key(), delta))
				continue;


			for(Service::Item * item: _invertedIndex[wm.key()])
			{
				resultsRef[item] += wm.value();
			}
		}
	}

	// Intersect the set of items references by the (referenced) words
	// This assusmes that there is at least one word (the query would not have
	// been started elsewise)
	QVector<QPair<Service::Item *, unsigned int>> finalResult;
	if (resultsPerWord.size() > 1)
	{
		// Get the smallest list for intersection (performance)
		unsigned int smallest=0;
		for (unsigned int i = 1; i < static_cast<unsigned int>(resultsPerWord.size()); ++i)
			if (resultsPerWord[i].size() < resultsPerWord[smallest].size())
				smallest = i;

		bool allResultsContainEntry;
		for (QMap<Service::Item *, unsigned int>::const_iterator r = resultsPerWord[smallest].begin(); r != resultsPerWord[smallest].cend(); ++r)
		{
			// Check if all results contain this entry
			allResultsContainEntry=true;
			unsigned int accMatches = resultsPerWord[smallest][r.key()];
			for (unsigned int i = 0; i < static_cast<unsigned int>(resultsPerWord.size()); ++i)
			{
				// Ignore itself
				if (i==smallest)
					continue;

				// If it is in: check next relutlist
				if (resultsPerWord[i].contains(r.key()))
				{
					// Accumulate matches
					accMatches += resultsPerWord[i][r.key()];
					continue;
				}

				allResultsContainEntry = false;
				break;
			}

			// If this is not common, check the next entry
			if (!allResultsContainEntry)
				continue;

			// Finally this match is common an can be put into the results
			finalResult.append(QPair<Service::Item *, unsigned int>(r.key(), accMatches));
		}
	}
	else // Else do it without intersction
	{
		for (QMap<Service::Item *, unsigned int>::const_iterator r = resultsPerWord[0].begin(); r != resultsPerWord[0].cend(); ++r)
			finalResult.append(QPair<Service::Item *, unsigned int>(r.key(), r.value()));
	}

	// Sort em by relevance
	std::sort(finalResult.begin(), finalResult.end(),
			  [&](QPair<Service::Item *, unsigned int> x, QPair<Service::Item *, unsigned int> y)
				{return x.second > y.second;});

	for (QPair<Service::Item *, unsigned int> pair : finalResult){
		res->append(pair.first);
	}
}
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
inline void IndexService::query(const QString&req, QVector<Service::Item*>*res) const {
	_search->query(req, res);
}

/**************************************************************************/
void IndexService::setSearchType(IndexService::SearchType T)
{
	_searchType = T;
	switch (T) {
	case IndexService::SearchType::Exact:
		delete _search;
		_search = new ExactMatchSearchImpl(_index);
//		qDebug() << "Set searchtype to ExactMatch";
		break;
	case IndexService::SearchType::WordMatch:
		delete _search;
		_search = new WordMatchSearchImpl(_index);
//		qDebug() << "Set searchtype to WordMatch";
		break;
	case IndexService::SearchType::Fuzzy:
		delete _search;
		_search = new FuzzySearchImpl(_index);
//		qDebug() << "Set searchtype to Fuzzy";
		break;
	}
}

/**************************************************************************/
void IndexService::prepareSearch()
{
	_search->prepare();
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
