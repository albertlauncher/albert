// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>
#include "item.h"
#include "matchcompare.h"
using namespace std;


/** ***************************************************************************/
map<QString, double> Core::MatchCompare::order;

/** ***************************************************************************/
bool Core::MatchCompare::operator()(const pair<shared_ptr<Item>, short> &lhs,
                                  const pair<shared_ptr<Item>, short> &rhs) {
    // Compare urgency
    if (lhs.first->urgency() != rhs.first->urgency())
        return lhs.first->urgency() > rhs.first->urgency();

    // Compare usage scores
    const map<QString,double>::iterator &lit = order.find(lhs.first->id());
    const map<QString,double>::iterator &rit = order.find(rhs.first->id());
    if (lit==order.cend()) // |- lhs zero
        if (rit==order.cend()) // |- rhs zero
            return lhs.second > rhs.second; // Compare match score
        else // |- rhs > 0
            return false; // lhs==0 && rhs>0 implies lhs<rhs implies !(lhs>rhs)
    else
        if (rit==order.cend())
            return true; // lhs>0 && rhs=0 implies lhs>rhs
        else
            return lit->second > rit->second; // Both usage scores available, return lhs>rhs
}


/** ***************************************************************************/
void Core::MatchCompare::update() {
    order.clear();

    // Update the results ranking
    QSqlQuery query;
    query.exec("SELECT t.itemId AS id, SUM(t.score) AS usageScore "
               "FROM ( "
               " SELECT itemId, 1/max(julianday('now')-julianday(timestamp),1) AS score "
               " FROM usages "
               " WHERE itemId<>'' "
               ") t "
               "GROUP BY t.itemId");
    while (query.next())
        MatchCompare::order.emplace(query.value(0).toString(),
                                    query.value(1).toDouble());
}
