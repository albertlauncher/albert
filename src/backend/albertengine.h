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
// Services
#include "abstractserviceprovider.h"
#include "abstractindexprovider.h"

using std::vector;
using std::map;

class AlbertEngine
{
public:
	explicit AlbertEngine();
	~AlbertEngine();

	void buildIndex();
	std::vector<AbstractServiceProvider::AbstractItem *> query(const std::string &req);
	static AlbertEngine* instance();

private:
	static AlbertEngine *_instance;
	vector<AbstractServiceProvider::AbstractItem*> _result;
	AbstractServiceProvider        *_websearch;
	AbstractServiceProvider        *_calculator;
	vector<AbstractIndexProvider*> _indizes;
};

#endif // ALBERTENGINE_H
