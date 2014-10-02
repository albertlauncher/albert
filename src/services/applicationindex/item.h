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

#ifndef ITEM_H
#define ITEM_H

#include "../index.h"
#include "applicationindex.h"

#include <QString>
#include <QDataStream>


/**************************************************************************/
class ApplicationIndex::Item : public Service::Item
{
	friend class ApplicationIndex;
public:
	inline QString title()            const override {return _name;}
	QIcon          icon()             const override;
	inline QString infoText()         const override {return _info;}
	inline QString complete()         const override {return _name;}
	void           action()           override;
	QString        actionText() const override;
	inline  qint64 lastAccess()       const;

protected:
	QString _name;
	QString _info;
	QString _iconName;
	QString _exec;
	bool    _term;

	// Serialization
	friend QDataStream &operator<<(QDataStream &out, const ApplicationIndex::Item &item);
	friend QDataStream &operator>>(QDataStream &in, ApplicationIndex::Item &item);
};

#endif // ITEM_H
