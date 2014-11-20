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
#include <QFileInfo>

class FileIndex::Item : public Service::Item
{
	friend class FileIndex;
	static const QMimeDatabase mimeDb;

public:
	Item(){}
	Item(QFileInfo fi) : _fileInfo(fi){}
	~Item(){}

	inline QString title() const override {return _fileInfo.fileName();}
	inline QString complete() const override {return _fileInfo.fileName();}
	inline QString infoText() const override {return _fileInfo.canonicalFilePath();}
	QIcon icon() const override;

	void    action() override;
	QString actionText() const override;
	void    altAction() override;
	QString altActionText() const override;

protected:
	QFileInfo _fileInfo;

	// Serialization
	void serialize (QDataStream &out) const override;
	void deserialize (QDataStream &in) override;
};
#endif // FILEITEM_H
