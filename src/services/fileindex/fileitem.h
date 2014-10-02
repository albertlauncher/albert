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

#ifndef FILEITEM_H
#define FILEITEM_H

#include "fileindex.h"
#include <QMimeDatabase>
#include <QString>
#include <QDataStream>
#include <QIcon>

/**************************************************************************/
class FileIndex::Item : public Index::Item
{
	friend class FileIndex;
	static const QMimeDatabase mimeDb;

public:
	Item(){}
	~Item(){}

	inline QString title()            const override {return _name;} //TODO
	inline QString complete()         const override {return _name;}
	inline QString infoText()         const override {return _path;}
	void           action()                 override;
	QString        actionText()       const override;
	QIcon          icon()             const override;

protected:
	QString _name;
	QString _path;

	friend QDataStream &operator<<(QDataStream &out, const FileIndex::Item &item);
	friend QDataStream &operator>>(QDataStream &in, FileIndex::Item &item);
};

#endif // FILEITEM_H
