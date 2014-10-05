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

#include "prefixmatchsearch.h"

/**************************************************************************/
Index::ExactMatchSearchImpl::ExactMatchSearchImpl(Index *p) : SearchImpl(p)
{

}

/**************************************************************************/
void Index::ExactMatchSearchImpl::query(const QString &req, QVector<Service::Item *> *res) const {
	QVector<Service::Item *>::const_iterator lb, ub;
	lb =  std::lower_bound (_parent->_index.cbegin(), _parent->_index.cend(), req, Index::CaseInsensitiveCompare());
	ub =  std::upper_bound (_parent->_index.cbegin(), _parent->_index.cend(), req, Index::CaseInsensitiveComparePrefix());
	while (lb!=ub)
		res->push_back(*(lb++));
}
