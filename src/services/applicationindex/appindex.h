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

#ifndef APPLICATIONINDEX_H
#define APPLICATIONINDEX_H

#include "indexservice.h"
#include "singleton.h"

class ApplicationIndex : public IndexService, public Singleton<ApplicationIndex>
{
	friend class Singleton<ApplicationIndex>;

public:
	class Item;

	~ApplicationIndex();

	QWidget* widget() override;
	void initialize() override;
	QDataStream& serialize (QDataStream &out) const override;
	QDataStream& deserialize (QDataStream &in) override;

protected:
	ApplicationIndex(){}
	void buildIndex() override;
};

#endif // APPLICATIONINDEX_H
