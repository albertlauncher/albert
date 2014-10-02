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

#ifndef ALBERTENGINE_H
#define ALBERTENGINE_H

#include "services/service.h"
#include <QVector>
#include <QString>
#include <QAbstractListModel>
#include <QStandardPaths>

class AlbertEngine : public QAbstractListModel
{
	Q_OBJECT

	struct ATimeCompare	{
		bool operator()( Service::Item const* lhs, Service::Item  const* rhs ) const {
			return lhs->lastAccess() > rhs->lastAccess();
		}
	};

public:
	AlbertEngine();
	~AlbertEngine();

	void query(const QString &req);
	void save(const QString& = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db") const;
	void load(const QString& = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/albert.db");

	// Modelstuff
	void     clear();
	void     action( const QModelIndex & index);
	void     altAction( const QModelIndex & index);
	void     ctrlAction( const QModelIndex & index);
	int      rowCount(const QModelIndex & = QModelIndex()) const override;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;


private:
//	WebSearch *_websearch;
//	Calculator *_calculator;
	QVector<Service*>        _modules;
	QVector<Service::Item *> _data;
};

#endif // ALBERTENGINE_H
