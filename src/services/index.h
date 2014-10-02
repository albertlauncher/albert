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

#ifndef INDEX_H
#define INDEX_H

#include "services/service.h"
#include <QString>
#include <QVector>

/**************************************************************************/
struct Index : public Service
{
	class SearchImpl;

	QVector<Service::Item*> _index;
	SearchImpl              *_search;

public:
	Index();
	virtual ~Index();
	enum class SearchType {Exact, WordMatch, Fuzzy};

	void query(const QString &req, QVector<Service::Item*> *res) const noexcept override;
	void setSearchType(SearchType);
};



#endif // INDEX_H
