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

#include "websearchitem.h"
#include <chrono>
#include <QClipboard>
#include <QGuiApplication>
#include <QDesktopServices>
#include <QUrl>

/**************************************************************************/
void WebSearch::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	QDesktopServices::openUrl(QUrl(QString(_url).replace("%s", _searchTerm)));
}

/**************************************************************************/
QString WebSearch::Item::actionText() const
{
	return QString("Visit '%1'.").arg(QString(_url).replace("%s", _searchTerm));
}

/**************************************************************************/
void WebSearch::Item::altAction()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	QGuiApplication::clipboard()->setText(QString(_url).replace("%s", _searchTerm));
}

/**************************************************************************/
QString WebSearch::Item::altActionText() const
{
	return QString("Copy '%1' to clipboard.").arg(QString(_url).replace("%s", _searchTerm));
}

/**************************************************************************/
QIcon WebSearch::Item::icon() const
{
	return QIcon(_iconPath);
}

/**************************************************************************/
void WebSearch::Item::serialize(QDataStream &out) const
{
	out << _name << _url << _shortcut << _iconPath << _lastAccess;
}

/**************************************************************************/
void WebSearch::Item::deserialize(QDataStream &in)
{
	in >> _name >> _url >> _shortcut >> _iconPath >> _lastAccess;
}
