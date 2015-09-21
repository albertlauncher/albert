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
#include <QString>
#include <QAbstractItemModel>
#include "interfaces/iitem.h"
#include "interfaces/iquery.h"

class Query final : public QAbstractItemModel, public IQuery
{
	Q_OBJECT

    /** ****************************************************************************
     * @brief The TreeItem struct
     * The albert data model
     */
    struct TreeItem final
    { // Dont mess with the memory!
        TreeItem(IItem *d = nullptr, TreeItem *p = nullptr, std::vector<TreeItem*> *c = nullptr)
            : data(d), parent(p), children(c) {}
        ~TreeItem(){ // Recursively kill the tree
            if (children) {
                for (TreeItem *t : *children)
                    delete t;
                delete children;
            }
            delete data;
        }
        void lazyLoadChildren() {
            if (!children && data->hasChildren()) {
                children = new std::vector<TreeItem*>();
                for (IItem *i : data->children()) // Mem allocated
                    children->push_back(new TreeItem(i, this));
            }
        }
        IItem *data; // OWNER!!!
        TreeItem *parent;
        std::vector<TreeItem*> *children; //Lazy instantiation
    };

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
        inline bool operator()(const TreeItem *lhs, const TreeItem *rhs) const {
            return lhs->data->score() > rhs->data->score();
        }
    private:
        Query *_q;
    };

public:
    explicit Query(QString term);
    ~Query();

    /**
     * @brief Add a top-level/root node to the albert tree
     * The query takes the ownerhip of the node.
     * @param node The amount of error tolerance
     */
    void add(IItem *node);

    /**
     * @brief Returns the search term of this query
     */
    const QString& searchTerm() const;

    /**
     * @brief Exectute the action of the referenced node
     * @param index The node reference
     */
    void activate(const QModelIndex & index);

    // TODO
    //bool dynamicSearch() const { return _dynamicSort; }
    //void setDynamicSearch(bool b){ _dynamicSort = b; }


    // Todo mocec this to EM
    // QAbstractItemModel interface
    int rowCount(const QModelIndex & index = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    QModelIndex index(int row, int column = 0, const QModelIndex & index = QModelIndex()) const override;
    bool hasChildren(const QModelIndex & parent) const override;
    void sort(int column = 0, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    TreeItem* _rootItem;
    QString _searchTerm;
    bool _dynamicSort;
};
