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

#ifndef SERVICE_H
#define SERVICE_H

#include <QString>
#include <QVector>
#include <QIcon>
#include <chrono>

/**************************************************************************/
struct Service
{
	class Item;
	Service(){}
	virtual ~Service(){}
	virtual void query(const QString&, QVector<Item*>*) const noexcept = 0;
	virtual void save (const QString&) const = 0;
	virtual void load (const QString&) = 0;
};

/**************************************************************************/
struct Service::Item
{
	Item(){}
	virtual ~Item(){}

	virtual QString   title()      const = 0;
	virtual QIcon     icon()       const = 0;
	virtual QString   infoText()   const = 0;
	virtual QString   complete()   const = 0;
	virtual void      action()           = 0;
	virtual QString   actionText() const = 0;
	qint64            lastAccess() const {return _lastAccess;}

protected:
	qint64 _lastAccess;
};


#endif // SERVICE_H
