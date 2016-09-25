// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QAbstractTableModel>

#include <memory>
#include <vector>
using std::shared_ptr;
using std::vector;

namespace Websearch {

class SearchEngine;

class SearchEnginesModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    SearchEnginesModel(vector<shared_ptr<SearchEngine>> &index, QObject *parent = Q_NULLPTR)
        : QAbstractTableModel(parent), index_(index) {}

    /*
     * Implementation of extension interface
     */

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    bool insertRows (int position, int rows, const QModelIndex & parent = QModelIndex()) override;
    bool removeRows (int position, int rows, const QModelIndex & parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceRow, int srcRow, int cnt, const QModelIndex & dst, int destinationChild) override;

private:
    vector<shared_ptr<SearchEngine>> &index_;

    enum class Section{Enabled, Name, Trigger, URL, Count};

signals:

    void fallBackChanged();
};

}
