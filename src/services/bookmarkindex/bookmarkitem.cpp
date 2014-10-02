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

#include "bookmarkitem.h"
#include <chrono>
#include <QProcess>
#include <QDebug>


/**************************************************************************/
void BookmarkIndex::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();

//	switch (a) {
//	case Action::Enter:
//	case Action::Alt:
		QProcess::startDetached("xdg-open " + _url);
//		break;
//	case Action::Ctrl:
////		WebSearch::instance()->defaultSearch(_name);
//		break;
//	}
}

/**************************************************************************/
QString BookmarkIndex::Item::actionText() const
{
//	switch (a) {
//	case Action::Enter:
//	case Action::Alt:
		return "Visit '" + _title + "'";
//		break;
//	case Action::Ctrl:
////		return WebSearch::instance()->defaultSearchText(_name);
//		break;
//	}
//	// Will never happen
//	return "";
}

/**************************************************************************/
QIcon BookmarkIndex::Item::icon() const
{
	if (QIcon::hasThemeIcon(QString::fromLocal8Bit("favorites")))
		return QIcon::fromTheme(QString::fromLocal8Bit("favorites"));
	return QIcon::fromTheme(QString::fromLocal8Bit("unknown"));
}

/**************************************************************************/
QDataStream &operator<<(QDataStream &out, const BookmarkIndex::Item &item)
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
//	out << item._title
//		<< item._lastAccess
//		<< item._url;
//	return out;
}

/**************************************************************************/
QDataStream &operator>>(QDataStream &in, BookmarkIndex::Item &item)
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
//	exit(1);
//	in >> item._title
//			>> item._lastAccess
//			>> item._url;
//	return in;
}
