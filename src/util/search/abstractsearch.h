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

#ifndef ABSTRACTSEARCH_H
#define ABSTRACTSEARCH_H

#include "abstractservice.h"
#include <QObject>
#include <QList>
#include <QVector>

class AbstractIndex;

class AbstractSearch : public QObject
{
	Q_OBJECT
public:
	explicit AbstractSearch(){}
	explicit AbstractSearch(AbstractIndex*ref) : _ref(ref) {}
	virtual ~AbstractSearch(){}

	inline void setIndex(AbstractIndex* ref){_ref=ref;}
	inline const AbstractIndex* index(){return _ref;}

	virtual void query(const QString &req, QVector<Service::Item*> *res) const = 0;

protected:
	AbstractIndex* _ref;

public slots:
	virtual void buildIndex() = 0;

};

#endif // ABSTRACTSEARCH_H
