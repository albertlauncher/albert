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

#include <functional>
#include <QHash>
#include <QString>
#include <QList>

template<class T>
class AbstractSearch
{
public:
	AbstractSearch() = delete;
	explicit AbstractSearch(QHash<QString, T> *idx, std::function<QString(T)> f)
		: _index(idx), _textFunctor(f) {}
	virtual ~AbstractSearch(){}

	virtual void buildIndex() = 0;
	virtual QStringList find(const QString &req) const = 0; // BLEIBT SO NICHT !MT

protected:
	QHash<QString, T> *_index;
	std::function<QString(T)> _textFunctor;
};

#endif // ABSTRACTSEARCH_H
