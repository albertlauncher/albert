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

#ifndef ABSTRACTINDEX_H
#define ABSTRACTINDEX_H

#include "abstractservice.h"
#include "abstractsearch.h"
#include <QObject>

class AbstractIndex : public QObject
{
	Q_OBJECT
	friend class SearchWidget;

public:
	explicit AbstractIndex() : _search(nullptr){
		connect(this, &AbstractIndex::endBuildIndex, this, &AbstractIndex::indexChanged);
	}
	virtual ~AbstractIndex(){
		disconnect(this, &AbstractIndex::endBuildIndex, this, &AbstractIndex::indexChanged);
		delete _search;
		for(Service::Item *i : _index)
			delete i;
		_index.clear();

	}

	const QList<Service::Item*> &data(){return _index;}

	/**
	 * @brief setSearch
	 *
	 * Set the search used by the index query method. Takes ownership of search.
	 * Builds search index. Connects build slot to the signal emitted by the
	 * index.
	 * @param s The search to be set
	 */
	inline void setSearch(AbstractSearch *s){
		if (_search != nullptr){
			disconnect(this, &AbstractIndex::indexChanged,_search, &AbstractSearch::buildIndex);
			delete _search;
		}
		_search = s;
		_search->setIndex(this);
		_search->buildIndex();
		connect(this, &AbstractIndex::indexChanged,_search, &AbstractSearch::buildIndex);
	}
	inline const AbstractSearch *search(){return _search;}

protected:
	QList<Service::Item*> _index;
	AbstractSearch        *_search;

signals:
	void beginBuildIndex();
	void endBuildIndex();
	void indexChanged();

public slots:
	virtual void buildIndex() = 0;
};
#endif // ABSTRACTINDEX_H
