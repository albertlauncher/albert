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
map<QString, uint> Core::MatchCompare::order;

/** ***************************************************************************/
bool Core::MatchCompare::operator()(const pair<shared_ptr<Item>, uint> &lhs,
                                  const pair<shared_ptr<Item>, uint> &rhs) {
    // Compare urgency then score
    if (lhs.first->urgency() != rhs.first->urgency())
        return lhs.first->urgency() > rhs.first->urgency();
    return lhs.second > rhs.second; // Compare match score
}


/*****************************************************************************
 * @brief Core::MatchCompare::update
 * Update the usage score:
 * Score of a single usage is 1/(<age_in_days>+1).
 * Accumulate all scores groupes by itemId.
 * Normalize the scores to the range of UINT_MAX.
 */
void Core::MatchCompare::update() {
    order.clear();
    QSqlQuery query;
    query.exec("SELECT itemId, SUM(1/(julianday('now')-julianday(timestamp)+1)) AS score "
               "FROM usages WHERE itemId<>'' GROUP BY itemId ORDER BY score DESC");
    double max;
    if ( !query.next() )
        return;
    max = query.value(1).toDouble();

    do {
        MatchCompare::order.emplace(query.value(0).toString(),
                                    static_cast<uint>(query.value(1).toDouble()*UINT_MAX/max));
    } while (query.next());
}

/** ***************************************************************************/
const std::map<QString, uint> &Core::MatchCompare::usageScores() {
    return order;
}
