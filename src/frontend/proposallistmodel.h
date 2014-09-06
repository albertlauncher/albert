// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#ifndef PROPOSALLISTMODEL_H
#define PROPOSALLISTMODEL_H

#include <QAbstractListModel>
#include "abstractserviceprovider.h"


class ProposalListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	explicit ProposalListModel(QObject *parent = 0);
	void set(const std::vector<AbstractServiceProvider::AbstractItem*> &d);
	void clear();
	void action( const QModelIndex & index);

protected:
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
	int rowCount(const QModelIndex & = QModelIndex()) const override;

	std::vector<AbstractServiceProvider::AbstractItem*> _data;

};

#endif // PROPOSALLISTMODEL_H
