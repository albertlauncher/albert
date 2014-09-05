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

#include "index.h"
#include "genericmimeindex.h"

Index::Index()
{
	_indizes = {new GenericMimeIndex};
}

Index::~Index()
{
	for (auto &i : _indizes)
		delete i;
}

std::vector<AbstractItem *> Index::query(QString q)
{
	std::vector<AbstractItem *> results;
	for (auto i : _indizes) {
		std::vector<AbstractItem *> tmp = i->query(q);
		results.insert(results.end(), tmp.begin(), tmp.end());
	}
	return results;
	// TODO SORT
}

QWidget* Index::configWidget()
{
	return new QWidget;
}
