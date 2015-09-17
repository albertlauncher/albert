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
#include <QIcon>
#include <algorithm>



/** ****************************************************************************
 * @brief The UsageCompare struct
 * This funtion object compares two items. A n item is larger than others if
 * the usage couter is greater. In case of equality the lexicographical string
 * comparison of the title is used.
 */
struct UsageCompare
{
    UsageCompare() = delete;
    UsageCompare(Query *q) : _q (q){}

    inline bool operator()(const INode *lhs, const INode *rhs) const
    {
        if (lhs->usage() == rhs->usage())
            return QString::compare(lhs->name(_q), rhs->name(_q), Qt::CaseInsensitive) < 0;
        else
            return lhs->usage() > rhs->usage();
    }

private:
    Query *_q;
};



/** ***************************************************************************/
void Query::addResult(INode *result)
{
    if (_dynamicSort)
        throw "Not implemented yet.";
    else {
        beginInsertRows(QModelIndex(), _results.size(), _results.size());
        _results.append(result);
        endInsertRows();
    }
}



/** ***************************************************************************/
void Query::activate(const QModelIndex &index)
{
    if (index.isValid())
        static_cast<INode*>(index.internalPointer())->activate(this);
}



/** ***************************************************************************/
void Query::sort()
{
    beginResetModel();
    std::stable_sort(_results.begin(), _results.end(), UsageCompare(this));
    endResetModel();
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/


/** ***************************************************************************/
QModelIndex Query::index(int row, int column, const QModelIndex & parent) const
{
    // This is about getting the index of a child
    Q_UNUSED(column);

    if (parent.isValid()){
        // Node - get the child if it exists
        INode *parentNode = static_cast<INode*>(parent.internalPointer());
        if (parentNode->hasChildren()){
            QList<INode*> children = parentNode->children();
            if (row < children.size())
                return createIndex(row, 0, static_cast<void*>(children[row]));
        }
    } else if (row < _results.size()) {
        // Root - get the top level item if it exists
        return createIndex(row, 0, static_cast<void*>(_results[row]));
    }
    return QModelIndex();
}



/** ***************************************************************************/
QModelIndex Query::parent(const QModelIndex & index) const
{
    if (!index.isValid())
        return QModelIndex();

    INode *parentNode = (static_cast<INode*>(index.internalPointer()))->parent();

    // Parent of top level item (invalid index)
    if (parentNode == nullptr)
        return QModelIndex();

    // Parent of node
    return createIndex(_results.indexOf(parentNode), 0, parentNode);
}



/** ***************************************************************************/
int Query::rowCount(const QModelIndex & index) const
{
    if (index.isValid()){ // Node #children
        return static_cast<INode*>(index.internalPointer())->children().count();
    } else { // Root #toplevelitems
        return _results.size();
    }
}


/** ***************************************************************************/
int Query::columnCount(const QModelIndex &index) const
{
    // This is a tree of lists
    Q_UNUSED(index);
    return 1;
}



/** ***************************************************************************/
QVariant Query::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
        return static_cast<INode*>(index.internalPointer())->name(this);
    if (role == Qt::ToolTipRole)
        return static_cast<INode*>(index.internalPointer())->description(this);
    if (role == Qt::DecorationRole)
        return static_cast<INode*>(index.internalPointer())->icon();
    return QVariant();
}



/** ***************************************************************************/
Qt::ItemFlags Query::flags(const QModelIndex &index) const
{
    // Does root need flags?
    Q_UNUSED(index);
    return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}



/** ***************************************************************************/
bool Query::hasChildren(const QModelIndex & index) const
{
    // Root has children
    if (!index.isValid()) // Top level
        return (_results.size()==0) ? false : true;

    // Everybody else has to tell this on its own
    return static_cast<INode*>(index.internalPointer())->hasChildren();
}
