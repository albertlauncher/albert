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
#include "stdint.h"
#include "objects.h"
using std::shared_ptr;

class Query final : public QAbstractItemModel {
	Q_OBJECT

public:
    explicit Query(QString term) : _searchTerm(term), _dynamicSort(false) {}
	~Query(){}

    void addResults(const QList<shared_ptr<AlbertObject>> &&results);
    void addResult(shared_ptr<AlbertObject> &&result);
    void setDynamicSearch(bool b){ _dynamicSort = b; }
    bool dynamicSearch() const { return _dynamicSort; }
    const QString& searchTerm() const {return _searchTerm;}
    void sort();

    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool hasChildren(const QModelIndex & parent = QModelIndex()) const override;

private:
    QList<shared_ptr<AlbertObject>> _results;
    QString _searchTerm;
    bool _dynamicSort;
};
