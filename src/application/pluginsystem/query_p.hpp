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

#pragma once
#include <QDebug>
#include <QVariant>
#include <QMutex>
#include <QTimer>
#include <QMutexLocker>
#include <QAbstractItemModel>
#include <utility>
#include <algorithm>
#include <vector>
#include <memory>
using std::shared_ptr;
using std::unique_ptr;
#include "abstractobjects.hpp"
#include "roles.hpp"

struct TreeItem final
{
    TreeItem(shared_ptr<AlbertItem> data)
        : parent(nullptr), data(data), childrenAreLoaded(false) {}

    TreeItem *parent;
    shared_ptr<AlbertItem> data;
    std::vector<unique_ptr<TreeItem>> children;
    bool childrenAreLoaded;
};

struct Match {
    TreeItem item;
    short score;
};

/** ***************************************************************************/
class QueryPrivate final : public QAbstractItemModel
{
    Q_OBJECT
    friend class ExtensionManager;

public:
    /** ***********************************************************************/
    QueryPrivate(const QString &term)
        : searchTerm_(term), dynamicSort_(false) {
        // Use this when muoltithreadeing
//        QTimer::singleShot(50, this, &QueryPrivate::makeDynamic);
    }



    /** ***********************************************************************/
    void addMatch(shared_ptr<AlbertItem> item, short score) {
        QMutexLocker locker(&mutex_);
        if (dynamicSort_) {
            //beginInsertRows(...);
            throw "Not implemented yet.";
        } else {
            matches_.push_back({item, score});
        }
    }



//    /** ***********************************************************************/
//    void removeMatches(IExtension *ext) {
//        QMutexLocker locker(&mutex_);
//        if (dynamicSort_) {
//            //beginremoveRows(...);
//            throw "Not implemented yet.";
//        } else {
//            std::remove_if(matches_.begin(), matches_.end(),
//                           [&](const Match &m){ return m.item.data()->extension()==ext; });
//        }
//    }



    /** ***********************************************************************/
    void reset() {
        QMutexLocker locker(&mutex_);
        if (dynamicSort_) {
            beginResetModel();
            throw "Not implemented yet.";
            endResetModel();
        } else {
            isValid_ = false;
            matches_.clear();
        }
    }



    /** ***********************************************************************/
    void setValid(bool b = true) {
        QMutexLocker locker(&mutex_);
        isValid_ = b;
    }



    /** ***********************************************************************/
    bool isValid() {
        return isValid_;
    }



    /** ***********************************************************************/
    const QString &searchTerm() const {
        return searchTerm_;
    }



    /** ***********************************************************************/
    void activate(const QModelIndex & index) {
        if (index.isValid())
            static_cast<TreeItem*>(index.internalPointer())->data->activate();
    }



    /** ***********************************************************************/
    void makeDynamic() {
        QMutexLocker locker(&mutex_);
        beginResetModel();
        std::stable_sort(matches_.begin(), matches_.end(),
                         [](const Match &lhs, const Match &rhs) {
                            return lhs.score > rhs.score;
                         });
        endResetModel();
        dynamicSort_ = true;
    }



    /** ***********************************************************************/
    int rowCount(const QModelIndex & parent = QModelIndex()) const override {
        if (parent.isValid()) // Child level
            return static_cast<int>(static_cast<TreeItem*>(parent.internalPointer())->data->children().size());
        else // Root level
            return static_cast<int>(matches_.size());
    }



    /** ***********************************************************************/
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 1;
    }



    /** ***********************************************************************/
    QVariant data(const QModelIndex & index, int role) const override {
        if (index.isValid()) {
            TreeItem *ti = static_cast<TreeItem*>(index.internalPointer());
            switch (role) {
            case Roles::Text:
                return ti->data->text();
            case Roles::SubText:
                return ti->data->subtext();
            case Roles::IconPath:
                return ti->data->iconPath();
            case Roles::Actions: {
                QStringList actionTexts;
                for (ActionSPtr &action : ti->data->actions())
                    actionTexts.append(action->text());
                return actionTexts;
            }
            default:
                return QVariant();
            }
        }
        return QVariant();
    }



    /** ***********************************************************************/
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (index.isValid()) {
            TreeItem *ti = static_cast<TreeItem*>(index.internalPointer());
            switch (role) {
            case Roles::Activate: {
                int actionValue = value.toInt();
                if (0 <= actionValue && actionValue < static_cast<int>(ti->data->actions().size()))
                    ti->data->actions()[actionValue]->activate();
                else
                    ti->data->activate();
                return true;
            }
            default:
                return false;
            }
        }
        return false;
    }



    /** ***********************************************************************/
    Qt::ItemFlags flags(const QModelIndex & index) const override {
        // Does root need flags?
        Q_UNUSED(index)
        return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
    }



    /** ***********************************************************************/
    QModelIndex parent(const QModelIndex & index) const override {
        // Return if this index is invalid
        if (!index.isValid())
            return QModelIndex();

        // Get the parentnode if it exists
        TreeItem *parentNode = static_cast<TreeItem*>(index.internalPointer())->parent;
        if (parentNode == nullptr) // Result is top level item -> parent is invalid
            return QModelIndex();

        // Get the grandparentnode
        TreeItem *grandParentNode = parentNode->parent;
        if (grandParentNode == nullptr) { // Parent is on root level
            // Get the position of the item, Search in matches
            int row = static_cast<int>(
                        std::distance(matches_.begin(),
                                      std::find_if(matches_.begin(),
                                                   matches_.end(),
                                                   [&parentNode](const Match &m){
                                                       return &m.item==parentNode;
                                                   })
                                      )
                        );
            // Parent of node
            return createIndex(row, 0, static_cast<void*>(parentNode));
        }
        else { // Parent is a child itself
            // Get the position of the item
            int row = static_cast<int>(
                        std::distance(grandParentNode->children.begin(),
                                      std::find_if(grandParentNode->children.begin(),
                                                   grandParentNode->children.end(),
                                                   [&parentNode](const unique_ptr<TreeItem> &u){
                                                       return u.get()==parentNode;
                                                   })
                                      )
                        );

            // Parent of node
            return createIndex(row, 0, static_cast<void*>(parentNode));
        }
    }



    /** ***********************************************************************/
    QModelIndex index(int row, int column = 0, const QModelIndex & parent = QModelIndex()) const override {
        Q_UNUSED(column)

        if (parent.isValid()) { // Child level
            // Get parent treeitem
            TreeItem *parentNode = static_cast<TreeItem*>(parent.internalPointer());

            // If not done already, lazy load the children
            if (!parentNode->childrenAreLoaded)
                for (ItemSPtr &ch : parentNode->data->children()){
                    unique_ptr<TreeItem> uniqueTreeItem(new TreeItem(ch));
                    uniqueTreeItem->parent = parentNode;
                    parentNode->children.push_back(std::move(uniqueTreeItem));
                }

            // Get the child if it exists
            if (static_cast<size_t>(row) < parentNode->children.size())
                return createIndex(row, 0, static_cast<void*>(parentNode->children[row].get()));
            else
                return QModelIndex();
        }
        else { // Root level
            // Get the child if it exists
            if (static_cast<size_t>(row) < matches_.size())
                // have to const cast this. since this function is const, "this"
                // is const, matches is const, operator [] returns const, item
                // is const and can therefore not be casted to void
                return createIndex(row, 0, static_cast<void*>(const_cast<TreeItem*>(&matches_[row].item)));
            else
                return QModelIndex();
        }
    }



    /** ***********************************************************************/
    bool hasChildren(const QModelIndex & parent) const override {
        if (parent.isValid())
            return static_cast<TreeItem*>(parent.internalPointer())->data->hasChildren();
        else
            return matches_.size() != 0; // Root has always children
    }

private:
    QString searchTerm_;
    std::vector<Match> matches_;
    bool isValid_;
    QMutex mutex_;
    bool dynamicSort_;
};


