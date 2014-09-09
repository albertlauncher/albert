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

#include "abstractindexprovider.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <settings.h>

std::vector<AbstractServiceProvider::AbstractItem*> AbstractIndexProvider::query(const std::string &req)
{
	std::vector<AbstractIndexItem *>::const_iterator lb, ub;
	lb =  std::lower_bound (_index.cbegin(), _index.cend(), req, CaseInsensitiveCompare(Settings::instance()->locale()));
	ub =  std::upper_bound (_index.cbegin(), _index.cend(), req, CaseInsensitiveComparePrefix(Settings::instance()->locale()));
	return std::vector<AbstractItem *>(lb, ub);
}




