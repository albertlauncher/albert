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

#ifndef ALBERTENGINE_H
#define ALBERTENGINE_H

#include <vector>
#include <map>
#include "index.h"
#include "abstractitem.h"

using std::vector;
using std::map;

class AlbertEngine
{
	Index *_index;
	static AlbertEngine *_instance;
	vector<const AbstractItem * > _result;

public:
	explicit AlbertEngine();
	~AlbertEngine();
	static AlbertEngine* instance(){
		if (_instance == nullptr)
			_instance = new AlbertEngine;
		return _instance;
	}
	std::vector<AbstractItem *> query(const QString &req);
	void buildIndex();
};

#endif // ALBERTENGINE_H
