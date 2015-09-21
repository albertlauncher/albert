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

#include <QDebug>
#include <QIcon>
#include <algorithm>
#include "query.h"

/** ***************************************************************************/
Query::Query(QString term)
    : _rootItem(new TreeItem(nullptr, nullptr, new std::vector<TreeItem*>)),
      _searchTerm(term), _dynamicSort(false) {
}



/** ***************************************************************************/
Query::~Query() {
    delete _rootItem;
}



/** ***************************************************************************/
void Query::add(IItem *result) {
    // This takes ownership !
    if (_dynamicSort)
        throw "Not implemented yet.";
    else {
        beginInsertRows(QModelIndex(),
                        static_cast<int>(_rootItem->children->size()),
                        static_cast<int>(_rootItem->children->size()));
        _rootItem->children->push_back(new TreeItem(result));
        endInsertRows();
    }
}



/** ***************************************************************************/
const QString &Query::searchTerm() const {
    return _searchTerm;
}



/** ***************************************************************************/
void Query::activate(const QModelIndex &index) {
    if (index.isValid())
        static_cast<TreeItem*>(index.internalPointer())->data->activate();
}



/** ***************************************************************************/
void Query::sort(int column, Qt::SortOrder order) {
    Q_UNUSED(column);
    Q_UNUSED(order);
    beginResetModel();
    std::stable_sort(_rootItem->children->begin(), _rootItem->children->end(), UsageCompare(this));
    endResetModel();
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/


/** ***************************************************************************/
QModelIndex Query::index(int row, int , const QModelIndex & index) const {
    // Get node
    TreeItem *node;
    if (index.isValid()) // Node
        node = static_cast<TreeItem*>(index.internalPointer());
    else // Root
        node = _rootItem;

    // Get the child if it exists
    node->lazyLoadChildren();
    if (static_cast<size_t>(row) < node->children->size())
        return createIndex(row, 0, static_cast<void*>((*node->children)[row]));
    else
        return QModelIndex();
}



/** ***************************************************************************/
QModelIndex Query::parent(const QModelIndex & index) const {
    // Return if this index is invalid
    if (!index.isValid())
        return QModelIndex();

    // Get the parentnode if it exists
    TreeItem *parentNode = static_cast<TreeItem*>(index.internalPointer())->parent;
    if (parentNode == nullptr) // Result is top level item -> parent is invalid
        return QModelIndex();

    // Get the grandparentnode
    TreeItem *grandParentNode = parentNode->parent;
    if (grandParentNode == nullptr)
        grandParentNode = _rootItem;

    // Get the position of the item
    int row = static_cast<int>(
                std::distance(grandParentNode->children->begin(),
                              std::find(grandParentNode->children->begin(),
                                        grandParentNode->children->end(),
                                        parentNode)
                              )
                );

    // Parent of node
    return createIndex(row, 0, static_cast<void*>(parentNode));
}



/** ***************************************************************************/
int Query::rowCount(const QModelIndex & index) const {
    TreeItem* node = (index.isValid()) ? static_cast<TreeItem*>(index.internalPointer()) : _rootItem;
    return static_cast<int>(node->children->size());
}


/** ***************************************************************************/
int Query::columnCount(const QModelIndex &) const {
    return 1;
}



/** ***************************************************************************/
QVariant Query::data(const QModelIndex & index, int role) const {
    if (index.isValid())
        return static_cast<TreeItem*>(index.internalPointer())->data->data(role);
    return QVariant();
}



/** ***************************************************************************/
Qt::ItemFlags Query::flags(const QModelIndex &) const {
    // Does root need flags?
    return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}



/** ***************************************************************************/
bool Query::hasChildren(const QModelIndex & index) const {
    if (index.isValid())
        return static_cast<TreeItem*>(index.internalPointer())->data->hasChildren();
    else
        return _rootItem->children->size() != 0; // Root has always children
}
