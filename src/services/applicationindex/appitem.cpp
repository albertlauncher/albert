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

#include <QDebug>

/**************************************************************************/
void ApplicationIndex::Item::action(Mod mod)
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	switch (mod) {
	case Mod::None:
		if (_term)
			QProcess::startDetached("konsole -e " + _exec);
		else
			QProcess::startDetached(_exec);
		break;
	case Mod::Alt:
		if (_term)
			QProcess::startDetached("kdesu konsole -e " + _exec);
		else
			QProcess::startDetached("kdesu " + _exec);
	case Mod::Ctrl:
		WebSearch::instance()->defaultSearch(_name);
		break;
	}
}

/**************************************************************************/
QString ApplicationIndex::Item::actionText(Mod mod) const
{
	switch (mod) {
	case Mod::None:
		return QString("Start '%1'.").arg(_name);
		break;
	case Mod::Alt:
		return QString("Start '%1' as root.").arg(_name);
	case Mod::Ctrl:
		return WebSearch::instance()->defaultSearchText(_name);
		break;
	}
	// Will never happen
	return "";
}

/**************************************************************************/
QDataStream &ApplicationIndex::Item::serialize(QDataStream &out) const
{
	out << _lastAccess << _name << _exec << _iconName << _info << _term;
	return out;
}

/**************************************************************************/
QDataStream &ApplicationIndex::Item::deserialize(QDataStream &in)
{
	in >> _lastAccess >> _name >> _exec >> _iconName >> _info >> _term;
	return in;
}

/**************************************************************************/
QIcon ApplicationIndex::Item::icon() const
{
	if (QIcon::hasThemeIcon(_iconName))
		return QIcon::fromTheme(_iconName);
	return QIcon::fromTheme("unknown");
}
