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
#include <QProcess>
#include <QClipboard>
#include <QGuiApplication>
#include <QSettings>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

/**************************************************************************/
void WebSearch::Item::action(Mod mod)
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	switch (mod) {
	case Mod::None:
		QDesktopServices::openUrl(QUrl(QString(_url).replace("%s", _searchTerm)));
		break;
	case Mod::Alt:
	case Mod::Ctrl:
		QGuiApplication::clipboard()->setText(QString(_url).replace("%s", _searchTerm));
		break;
	}
}

/**************************************************************************/
QString WebSearch::Item::actionText(Mod mod) const
{
	switch (mod) {
	case Mod::None:
		return QString("Visit '%1'.").arg(QString(_url).replace("%s", _searchTerm));
		break;
	case Mod::Alt:
	case Mod::Ctrl:
		return QString("Copy '%1' to clipboard.").arg(QString(_url).replace("%s", _searchTerm));
		break;
	}
	// Will never happen
	return "";
}

/**************************************************************************/
QIcon WebSearch::Item::icon() const
{
	return QIcon(_iconName);
}

/**************************************************************************/
QDataStream &operator<<(QDataStream &out, const WebSearch::Item &item)
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
}

/**************************************************************************/
QDataStream &operator>>(QDataStream &in, WebSearch::Item &item)
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
}
