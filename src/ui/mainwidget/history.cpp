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

#include "history.h"

/**********************************************************************/
History::History()
{
	reset();
}

/**********************************************************************/
void History::saveSettings(QSettings &s) const
{
	s.setValue("maxHistory", _max);
}

/**********************************************************************/
void History::loadSettings(QSettings &s)
{
	_max = s.value("maxHistory", 32).toInt();
}

/**********************************************************************/
void History::serilizeData(QDataStream &out) const
{
//	out << _data;
}

/**********************************************************************/
void History::deserilizeData(QDataStream &in)
{
//	in >> _data;
}

/**********************************************************************/
void History::insert(QString s)
{
	// Remove dups
	int i = _data.indexOf(s);
	if (i != -1)
		_data.removeAt(i);
	_data.insert(0, s);
	if (_data.count() > _max)
		_data.removeLast();
	reset();
}

/**********************************************************************/
bool History::hasNext() const
{
	return _it != _data.end();
}

/**********************************************************************/
const QString &History::next()
{
	return *(_it++);
}

/**********************************************************************/
void History::reset()
{
	_it = _data.begin();
}
