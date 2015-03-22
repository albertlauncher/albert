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

#ifndef BOOKMARKITEM_H
#define BOOKMARKITEM_H

#include "bookmarkindex.h"
#include <QString>
#include <QIcon>
#include <QDataStream>

class BookmarkIndex::Item : public Service::Item
{
	friend class BookmarkIndex;

public:
	Item(){}
	~Item(){}

	inline QString title() const override {return _title;}
	inline QString complete() const override {return _title;}
	inline QString infoText() const override {return _url;}
	QIcon icon() const override;

	void    action() override;
	QString actionText() const override;
	void    altAction() override;
	QString altActionText() const override;

protected:
	QString _title;
	QString _url;

	// Serialization
	void serialize (QDataStream &out) const override;
	void deserialize (QDataStream &in) override;
};
#endif // BOOKMARKITEM_H
