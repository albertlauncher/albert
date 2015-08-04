// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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

#include "query.h"
#include <QDebug>
#include <algorithm>



/** ****************************************************************************
 * @brief The UsageCompare struct
 * This funtion object compares two items. A n item is larger than others if
 * the usage couter is greater. In case of equality the lexicographical string
 * comparison of the title is used.
 */
struct UsageCompare {
    UsageCompare() = delete;
    UsageCompare(Query *q) : _q (q){}

    inline bool operator()(const SharedObject &lhs, const SharedObject &rhs) const {
        if (lhs->usage() == rhs->usage())
            return QString::compare(lhs->name(), rhs->name(), Qt::CaseInsensitive) < 0;
        else
            return lhs->usage() > rhs->usage();
    }
private:
    Query *_q;
};



/** ***************************************************************************/
void Query::addResults(const QList<SharedObject> &results) {
    if (_dynamicSort)
        throw "Not implemented yet.";
    else {
        beginInsertRows(QModelIndex(), _results.size(), _results.size()+results.size()-1);
        _results.append(results);
        endInsertRows();
    }
}



/** ***************************************************************************/
void Query::addResult(SharedObject &result) {
    if (_dynamicSort)
        throw "Not implemented yet.";
    else {
        beginInsertRows(QModelIndex(), _results.size(), _results.size());
        _results.append(result);
        endInsertRows();
    }
}



/** ***************************************************************************/
int Query::rowCount(const QModelIndex &) const {
    return _results.size();
}



/** ***************************************************************************/
QVariant Query::data(const QModelIndex &index, int role) const {
    // Strip out modifiers
//    Qt::KeyboardModifiers mods = static_cast<Qt::KeyboardModifiers>(role & Qt::KeyboardModifierMask);//TODO 0.7
    role = role & ~Qt::KeyboardModifierMask;

	if (!index.isValid())
		return QVariant();
	if (role == Qt::DisplayRole)
        return _results[index.row()]->name();
	if (role == Qt::ToolTipRole)
        return _results[index.row()]->description();
    if (role == Qt::DecorationRole)
        return _results[index.row()]->icon();
//    if (role == Qt::UserRole)
//        _results[index.row()]->action(*this, mods);
//    if (role == Qt::UserRole + 1)
//        return _results[index.row()]->actionText(*this, mods);////TODO 0.7
    if (role == Qt::UserRole + 2)
        return _results[index.row()]->usage();
    return QVariant();
}



/** ***************************************************************************/
void Query::sort() {
    beginResetModel();
    std::stable_sort(_results.begin(), _results.end(), UsageCompare(this));
    endResetModel();
}
