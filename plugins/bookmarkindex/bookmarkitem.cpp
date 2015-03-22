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
#include "websearch/websearch.h"
#include <chrono>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

/**************************************************************************/
void BookmarkIndex::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
//	QDesktopServices::openUrl(QUrl(_url));
	QProcess::startDetached(QString("kstart --activate chromium %1").arg(QUrl(_url).toString()));
}

/**************************************************************************/
QString BookmarkIndex::Item::actionText() const
{
	return QString("Visit '%1'.").arg(_title);
}

/**************************************************************************/
void BookmarkIndex::Item::altAction()
{
	action();
}

/**************************************************************************/
QString BookmarkIndex::Item::altActionText() const
{
	return actionText();
}

/**************************************************************************/
QIcon BookmarkIndex::Item::icon() const
{
	if (QIcon::hasThemeIcon(QString::fromLocal8Bit("favorites")))
		return QIcon::fromTheme(QString::fromLocal8Bit("favorites"));
	return QIcon::fromTheme(QString::fromLocal8Bit("unknown"));
}

/**************************************************************************/
void BookmarkIndex::Item::serialize(QDataStream &out) const
{
	out << _lastAccess << _title << _url;
}

/**************************************************************************/
void BookmarkIndex::Item::deserialize(QDataStream &in)
{
	in >> _lastAccess >> _title >> _url;
}
