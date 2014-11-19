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

#ifndef FUZZYSEARCH_H
#define FUZZYSEARCH_H

#include "abstractsearch.h"

#include <QMap>

class FuzzySearch final : public AbstractSearch
{
public:
	explicit FuzzySearch(unsigned int q = 3, double d = 2)
		: FuzzySearch(nullptr, q, d){}
	explicit FuzzySearch(AbstractIndex *ref, unsigned int q = 3, double d = 2)
		: AbstractSearch(ref), _q(q), _delta(d){}
	virtual ~FuzzySearch(){}

	void buildIndex() override;
	void query(const QString &req, QVector<Service::Item*> *res) const override;

	// Length of the grams
	inline unsigned int q() const {return _q;}
	inline void setQ(unsigned int q){_q=q; buildIndex();}

	// Max allowed errors
	inline double delta() const {return _delta;}
	inline void setDelta(unsigned int d){_delta=d;}

private:
	// Map of words, containing their item references
	typedef QMap<QString, QSet<Service::Item*>> InvertedIndex;
	InvertedIndex _invertedIndex;

	// Map of qGrams, containing their word references and #occurences
	typedef QMap<QString, QMap<QString, unsigned int>> QGramIndex;
	QGramIndex _qGramIndex;

	unsigned int _q;
	double _delta;

	static bool checkPrefixEditDistance(const QString& prefix, const QString& str, unsigned int delta);
};

#endif // FUZZYSEARCH_H
