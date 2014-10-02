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

#include "fileitem.h"
#include <chrono>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

#include <QDebug>

/**************************************************************************/
const QMimeDatabase FileIndex::Item::mimeDb;

/**************************************************************************/
void FileIndex::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();

//	switch (a) {
//	case Action::Enter:

		QDesktopServices::openUrl(QUrl(_path, QUrl::StrictMode)); //TODO
//		break;
//	case Action::Alt:
//		QDesktopServices::openUrl(QUrl(QFileInfo(_path).absolutePath()));
//		break;
//	case Action::Ctrl:
////		WebSearch::instance()->defaultSearch(_name);
//		break;
//	}
}

/**************************************************************************/
QString FileIndex::Item::actionText() const
{
//	switch (a) {
//	case Action::Enter:
		return "Open '" + _name + "' with default application";
//		break;
//	case Action::Alt:
//		return "Open the folder containing '" + _name + "' in file browser";
//		break;
//	case Action::Ctrl:
////		return WebSearch::instance()->defaultSearchText(_name);
//		break;
//	}
//	// Will never happen
//	return "";
}

/**************************************************************************/
QIcon FileIndex::Item::icon() const
{
	QString iconName = mimeDb.mimeTypeForFile(_path).iconName();
	if (QIcon::hasThemeIcon(iconName))
		return QIcon::fromTheme(iconName);
	return QIcon::fromTheme(QString::fromLocal8Bit("unknown"));
}

/**************************************************************************/
QDataStream &operator<<(QDataStream &out, const FileIndex::Item &item)
{
	out << item._name
		<< item._lastAccess
		<< item._path;
	return out;
}

/**************************************************************************/
QDataStream &operator>>(QDataStream &in, FileIndex::Item &item)
{
	in >> item._name
			>> item._lastAccess
			>> item._path;
	return in;
}
