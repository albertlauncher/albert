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

#include "engine.h"
#include "services/websearch/websearch.h"
#include "services/fileindex/fileindex.h"
#include "services/calculator/calculator.h"
#include "services/bookmarkindex/bookmarkindex.h"
#include "services/appindex/appindex.h"
#include <QString>

/**********************************************************************/
Engine::Engine()
{
	// Load modules
	_modules.push_back(new WebSearch);
	_modules.push_back(new Calculator);
	_modules.push_back(new AppIndex);
	_modules.push_back(new BookmarkIndex);
	_modules.push_back(new FileIndex);

}

/**********************************************************************/
Engine::~Engine()
{
	for (Service *i : _modules)
		delete i;
}

/**************************************************************************/
void Engine::initialize()
{
	for (Service *i : _modules)
		i->initialize();
}

/**************************************************************************/
void Engine::saveSettings(QSettings &s) const
{
	for (Service *i: _modules)
		i->saveSettings(s);
}

/**************************************************************************/
void Engine::loadSettings(QSettings &s)
{
	for (Service *i: _modules)
		i->loadSettings(s);
}

/**************************************************************************/
void Engine::serilizeData(QDataStream &out) const
{
	for (Service *i: _modules)
		i->serilizeData(out);
}

/**************************************************************************/
void Engine::deserilizeData(QDataStream &in)
{
	for (Service *i: _modules)
		i->deserilizeData(in);
}

/**********************************************************************/
void Engine::query(const QString &req)
{
	_requestString = req.trimmed();
	beginResetModel();
	_data.clear();
	if (!_requestString.isEmpty()){
		for (Service *i: _modules)
			i->query(_requestString, &_data);
		if (_data.isEmpty())
			for (Service *i: _modules)
				i->queryFallback(_requestString, &_data);
		std::stable_sort(_data.begin(), _data.end(), Service::Item::ATimeCompare());
	}
	endResetModel();
}

/**************************************************************************/
QVariant Engine::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole)
		return _data[index.row()]->title();

	if (role == Qt::DecorationRole)
		return _data[index.row()]->icon();

	if (role == Qt::ToolTipRole)
		return _data[index.row()]->infoText();

	if (role == Qt::UserRole)
		return _data[index.row()]->complete();


	if (role == Qt::UserRole+10)
		return _data[index.row()]->actionText();

	if (role == Qt::UserRole+11)
		return _data[index.row()]->altActionText();

	if (role == Qt::UserRole+12)
		return static_cast<WebSearch*>(_modules[0])->defaultSearchText(_requestString); // Todo implement a mechaninsm which let the modules offer global actions


	if (role == Qt::UserRole+20)
		_data[index.row()]->action();

	if (role == Qt::UserRole+21)
		_data[index.row()]->altAction();

	if (role == Qt::UserRole+22)
		static_cast<WebSearch*>(_modules[0])->defaultSearch(_requestString);

	return QVariant();
}

/**************************************************************************/
int Engine::rowCount(const QModelIndex&) const
{
	return _data.size();
}
