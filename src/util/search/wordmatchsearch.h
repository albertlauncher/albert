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

#ifndef WORDMATCHSEARCH_H
#define WORDMATCHSEARCH_H

#include "abstractsearch.h"

#include <QList>
#include <QString>
#include <QVector>
#include <QPair>
#include <QSet>

class WordMatchSearch final : public AbstractSearch
{
public:
	explicit WordMatchSearch() : WordMatchSearch(nullptr){}
	explicit WordMatchSearch(AbstractIndex *ref) : AbstractSearch(ref){}
	virtual ~WordMatchSearch(){}

	void buildIndex() override;
	void query(const QString &req, QVector<Service::Item*> *res) const override;

private:
	class CaseInsensitiveCompare;
	class CaseInsensitiveComparePrefix;

	typedef QPair<QString, QSet<Service::Item*>> Posting;
	typedef QVector<Posting> InvertedIndex;

	QVector<Posting> _invertedIndex;
};

#endif // WORDMATCHSEARCH_H
