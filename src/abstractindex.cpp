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

#include "abstractindex.h"
#include "abstractindexitem.h"

#include <string>
using std::string;


std::vector<AbstractItem *> AbstractIndex::query(QString req)
{
  QString reqlo = req.toLower();
  std::vector<AbstractIndexItem *>::const_iterator it, first, last, lb;
  std::iterator_traits<std::vector<AbstractIndexItem *>::const_iterator>::difference_type count, step;

  // lower bound
  first = _index.cbegin();
  last = _index.cend();
  count = distance(first,last);
  while (count>0) {
	it = first;
	step=count/2;
	advance (it,step);
	if (strncmp(reqlo.toStdString().c_str(), (*it)->title().toLower().toStdString().c_str(), reqlo.size()) > 0) {
	  first=++it;
	  count-=step+1;
	}
	else
	  count=step;
  }
  lb = it;

  // upper bound
  while (it != _index.end() && reqlo.toStdString().compare(0, string::npos, (*it)->title().toLower().toStdString(),0,reqlo.size()) == 0){
	++it;
  }

  return std::vector<AbstractItem *>(lb, it);
}
