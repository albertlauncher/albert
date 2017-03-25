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

#pragma once
#include <QAbstractTableModel>
#include <memory>
#include <vector>
#include "externalextension.h"

namespace ExternalExtensions {

class ExternalExtensionsModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    ExternalExtensionsModel(const std::vector<std::unique_ptr<ExternalExtension>> &exts, QObject *parent = Q_NULLPTR)
        : QAbstractTableModel(parent), externalExtensions_(exts) {}

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    void onActivated(const QModelIndex &);

private:

    const std::vector<std::unique_ptr<ExternalExtension>> &externalExtensions_;
    enum class Section{Name, Trigger, Path, Count};
};

}
