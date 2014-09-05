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

#ifndef GENERICMIMEITEM_H
#define GENERICMIMEITEM_H

#include <QString>
#include "abstractindexitem.h"

class GenericMimeItem : public AbstractIndexItem
{
public:
	GenericMimeItem() = delete;
	GenericMimeItem(QString title, QString uri)
		: AbstractIndexItem(title, uri) {}
	~GenericMimeItem() {}
	void action(int) override;
	QString actionText(int) override;
};

#endif // GENERICMIMEITEM_H
