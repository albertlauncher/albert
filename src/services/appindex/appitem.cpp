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

#include "appitem.h"
#include "websearch/websearch.h"
#include <chrono>
#include <QProcess>
#include <QDirIterator>
#include <QDebug>

/**************************************************************************/
void AppIndex::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	if (_term)
		QProcess::startDetached("konsole -e " + _exec);
	else
		QProcess::startDetached(_exec);

}

/**************************************************************************/
QString AppIndex::Item::actionText() const
{
	return QString("Start '%1'.").arg(_name);
}

/**************************************************************************/
void AppIndex::Item::altAction()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	if (_term)
		QProcess::startDetached("kdesu konsole -e " + _exec);
	else
		QProcess::startDetached("kdesu " + _exec);
}

/**************************************************************************/
QString AppIndex::Item::altActionText() const
{
	return QString("Start '%1' as root.").arg(_name);
}

/**************************************************************************/
void AppIndex::Item::serialize(QDataStream &out) const
{
	out << _lastAccess << _name << _exec << _iconName << _info << _term;
}

/**************************************************************************/
void AppIndex::Item::deserialize(QDataStream &in)
{
	in >> _lastAccess >> _name >> _exec >> _iconName >> _info >> _term;
}
