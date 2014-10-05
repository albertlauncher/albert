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
#include "searchimpl.h"
#include "fuzzysearch.h"
#include "prefixmatchsearch.h"
#include "prefixmatchanywordsearch.h"
#include <algorithm>
#include <QMap>
#include <QSet>
#include <QString>
#include <QDebug>


/**************************************************************************/
Index::Index()
{
	_search =  new ExactMatchSearchImpl(this);
	_searchType = Index::SearchType::Exact;
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
	_searchType = T;
	switch (T) {
	case Index::SearchType::Exact:
		if (!dynamic_cast<ExactMatchSearchImpl*>(_search)){
			delete _search;
			_search = new ExactMatchSearchImpl(this);
			qDebug() << "Set searchtype to ExactMatch";
		}
		break;
	case Index::SearchType::WordMatch:
		if (!dynamic_cast<WordMatchSearchImpl*>(_search)){
			delete _search;
			_search = new WordMatchSearchImpl(this);
			qDebug() << "Set searchtype to WordMatch";
		}
		break;
	case Index::SearchType::Fuzzy:
		if (!dynamic_cast<FuzzySearchImpl*>(_search)){
			delete _search;
			_search = new FuzzySearchImpl(this);
			qDebug() << "Set searchtype to Fuzzy";
		}
		break;
	}
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
