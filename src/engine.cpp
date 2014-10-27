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
#include <QDebug>
#include <QTime>

/**********************************************************************/
Engine::Engine()
{
	// Load modules
	_modules.push_back(WebSearch::instance());
	_modules.push_back(Calculator::instance());
	_modules.push_back(AppIndex::instance());
	_modules.push_back(BookmarkIndex::instance());
	_modules.push_back(FileIndex::instance());

}

/**********************************************************************/
Engine::~Engine()
{
}

/**************************************************************************/
void Engine::initialize()
{
	for (Service *i : _modules)
		i->initialize();
}

/**************************************************************************/
QDataStream &Engine::serialize(QDataStream &out) const
{
	for (Service *i: _modules)
		i->serialize(out);
	return out;
}

/**************************************************************************/
QDataStream &Engine::deserialize(QDataStream &in)
{
	for (Service *i: _modules)
		i->deserialize(in);
	return in;
}

/**********************************************************************/
void Engine::query(const QString &req)
{
	beginResetModel();
	_data.clear();
	for (Service *i: _modules)
	{
		QTime t;
		t.start();
		i->query(req, &_data);
		qDebug() << t.elapsed() << "µs";
	}
	if (_data.isEmpty())
		WebSearch::instance()->queryAll(req, &_data);

	// Sort them by atime
	std::stable_sort(_data.begin(), _data.end(), Service::Item::ATimeCompare());
	endResetModel();
}


/**************************************************************************/
void Engine::clear()
{
	beginResetModel();
	_data.clear();
	endResetModel();
}

/**************************************************************************/
void Engine::action(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action(Service::Item::Mod::None);
}

/**************************************************************************/
void Engine::altAction(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action(Service::Item::Mod::Alt);
}

/**************************************************************************/
void Engine::ctrlAction(const QModelIndex &index)
{
	if (rowCount() != 0)
		_data[index.isValid()?index.row():0]->action(Service::Item::Mod::Ctrl);
}

/**************************************************************************/
QVariant Engine::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	//	EnterText = Qt::UserRole;
	//	AltText  = Qt::UserRole+1;
	//	CtrlText  = Qt::UserRole+2;
	//	InfoText  = Qt::UserRole+3;
	//	Completion = Qt::UserRole+4;
	//	ExecutionNormal = Qt::UserRole+5;
	//	ExecutionAlt = Qt::UserRole+6;
	//	ExecutionCTRL = Qt::UserRole+7;

	if (role == Qt::DisplayRole)
		return _data[index.row()]->title();

	if (role == Qt::DecorationRole)
		return _data[index.row()]->icon();

	if (role == Qt::UserRole)
		return _data[index.row()]->actionText(Service::Item::Mod::None);

	if (role == Qt::UserRole+1)
		return _data[index.row()]->actionText(Service::Item::Mod::Alt);

	if (role == Qt::UserRole+2 )
		return _data[index.row()]->actionText(Service::Item::Mod::Ctrl);

	if (role == Qt::UserRole+3)
		return _data[index.row()]->infoText();

	if (role == Qt::UserRole+4)
		return _data[index.row()]->complete();

	if (role == Qt::UserRole+5)
		_data[index.row()]->action(Service::Item::Mod::None);

	if (role == Qt::UserRole+6)
		_data[index.row()]->action(Service::Item::Mod::Alt);

	if (role == Qt::UserRole+7)
		_data[index.row()]->action(Service::Item::Mod::Ctrl);

	return QVariant();
}

/**************************************************************************/
int Engine::rowCount(const QModelIndex&) const
{
	return _data.size();
}
