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

#ifndef BOOKMARKINDEX_H
#define BOOKMARKINDEX_H

#include "../index.h"
#include "../../singleton.h"

class BookmarkIndex : public Index, public Singleton<BookmarkIndex>
{
	friend class Singleton<BookmarkIndex>;

public:
	class Item;

	~BookmarkIndex();

	QWidget* widget() const override;
	void initialize() override;
	QDataStream& serialize (QDataStream &out) const override;
	QDataStream& deserialize (QDataStream &in) override;

protected:
	BookmarkIndex(){}
	void buildIndex() override;
};

#endif // BOOKMARKINDEX_H
