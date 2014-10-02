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

#include "applicationitem.h"
#include <chrono>
#include <QProcess>

#include <QDebug>

/**************************************************************************/
void ApplicationIndex::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();

		if (_term)
			QProcess::startDetached("konsole -e " + _exec);
		else
			QProcess::startDetached(_exec);


		//		break;
//	case Action::Alt:
//		if (_term)
//			QProcess::startDetached("kdesu konsole -e " + _exec);
//		else
//			QProcess::startDetached("kdesu " + _exec);
//		break;
//	case Action::Ctrl:
////		WebSearch::instance()->defaultSearch(_name);
//		break;
//	}
}

/**************************************************************************/
QString ApplicationIndex::Item::actionText() const
{
		return "Start " + _name;
//		break;
//	case Action::Alt:
//		return "Start " + _name + " as root";
//		break;
//	case Action::Ctrl:
////		return WebSearch::instance()->defaultSearchText(_name);
//		break;
//	}
//	// Will never happen
//	return "";
}


/**************************************************************************/
QIcon ApplicationIndex::Item::icon() const
{
	if (QIcon::hasThemeIcon(_iconName))
		return QIcon::fromTheme(_iconName);
	return QIcon::fromTheme("unknown");
}

/**************************************************************************/
QDataStream &operator<<(QDataStream &out, const ApplicationIndex::Item &item)
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
//	out << item._name
//		<< item._exec
//		<< item._iconName
//		<< item._info
//		<< item._lastAccess
//		<< item._term;
//	return out;
}

/**************************************************************************/
QDataStream &operator>>(QDataStream &in, ApplicationIndex::Item &item)
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
//	in >> item._name
//			>> item._exec
//			>> item._iconName
//			>> item._info
//			>> item._lastAccess
//			>> item._term;
//	return in;
}
