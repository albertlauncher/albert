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


/****************************************************************************///
void Query::addResults(const QList<QueryResult> &&results)
{
	beginInsertRows(QModelIndex(), _results.size(), _results.size()+results.size()-1);
	_results.append(results);
	endInsertRows();
}

/****************************************************************************///
void Query::addResult(QueryResult &&result)
{
	beginInsertRows(QModelIndex(), _results.size(), _results.size());
	_results.append(result);
//	qDebug() << _results.size();
	endInsertRows();
}

/****************************************************************************///
QVariant Query::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::DisplayRole)
		return _results[index.row()].text();
	if (role == Qt::ToolTipRole)
		return _results[index.row()].subtext();
//	if (role == Qt::DecorationRole)  // TODO UNCOMMENT
//		return _results[index.row()].icon();

//	if (role == Qt::UserRole)
//	if (role == Qt::UserRole+10)
//	if (role == Qt::UserRole+11)
	return QVariant();
}

/****************************************************************************///
int Query::rowCount(const QModelIndex&) const
{
	return _results.size();
}

/****************************************************************************///
void Query::run(const QModelIndex &)
{
	// TODO: Complete this if queryresult interface is known

}
