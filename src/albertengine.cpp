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

#include "albertengine.h"
//#include "services/websearch/websearch.h"
#include "services/fileindex/fileindex.h"
#include "services/calculator/calculator.h"
#include "services/bookmarkindex/bookmarkindex.h"
#include "services/applicationindex/applicationindex.h"

/**********************************************************************/
AlbertEngine::AlbertEngine()
{
	_modules.push_back(new Calculator);
	_modules.push_back(new ApplicationIndex);
	_modules.push_back(new FileIndex);
	_modules.push_back(new BookmarkIndex);
//	_websearch = new WebSearch;
}

/**********************************************************************/
AlbertEngine::~AlbertEngine()
{
	for (Service *i : _modules)
		delete i;
//	delete _calculator;
//	delete _websearch;

}

/**********************************************************************/
void AlbertEngine::query(const QString &req)
{
	beginResetModel();

	_data.clear();
//	_calculator->query(req, res);
//	_websearch->query(req, res);
	for (Service *i: _modules)
		i->query(req, &_data);
//	if (res->empty())
//		_websearch->queryAll(req, res);

	// Sort them by atime
	std::sort(_data.begin(), _data.end(), ATimeCompare());
	endResetModel();
}

/**************************************************************************/
void AlbertEngine::save(const QString & f) const
{
	for (Service *i: _modules)
		i->save(f);
}

/**************************************************************************/
void AlbertEngine::load(const QString &f)
{
//	for (Service *i: _modules) // TODO
//		i->load(f);
}

/**************************************************************************/
void AlbertEngine::clear()
{
	beginResetModel();
	_data.clear();
	endResetModel();
}

/**************************************************************************/
void AlbertEngine::action(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action();
}

/**************************************************************************/
void AlbertEngine::altAction(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action();
}

/**************************************************************************/
void AlbertEngine::ctrlAction(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action();
}

/**************************************************************************/
QVariant AlbertEngine::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	//	EnterText = Qt::UserRole;
	//	AltText  = Qt::UserRole+1;
	//	CtrlText  = Qt::UserRole+2;
	//	InfoText  = Qt::UserRole+3;
	//	Completion = Qt::UserRole+4;

	if (role == Qt::DisplayRole)
		return _data[index.row()]->title();

	if (role == Qt::DecorationRole)
		return _data[index.row()]->icon();

	if (role == Qt::UserRole)
		return _data[index.row()]->actionText();

	if (role == Qt::UserRole+1)
		return _data[index.row()]->actionText();

	if (role == Qt::UserRole+2 )
		return _data[index.row()]->actionText();

	if (role == Qt::UserRole+3)
		return _data[index.row()]->infoText();

	if (role == Qt::UserRole+4)
		return _data[index.row()]->complete();

	return QVariant();
}

/**************************************************************************/
int AlbertEngine::rowCount(const QModelIndex&) const
{
	return _data.size();
}
