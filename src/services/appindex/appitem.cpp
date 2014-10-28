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
void AppIndex::Item::action(Mod mod)
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
QString AppIndex::Item::actionText(Mod mod) const
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
QDataStream &AppIndex::Item::serialize(QDataStream &out) const
{
	out << _lastAccess << _name << _exec << _iconName << _info << _term;
	return out;
}

/**************************************************************************/
QDataStream &AppIndex::Item::deserialize(QDataStream &in)
{
	in >> _lastAccess >> _name >> _exec >> _iconName >> _info >> _term;
	return in;
}

/**************************************************************************/
QIcon AppIndex::Item::icon() const
{
	/* Icons and themes are looked for in a set of directories. By default,
	 * apps should look in $HOME/.icons (for backwards compatibility), in
	 * $XDG_DATA_DIRS/icons and in /usr/share/pixmaps (in that order).
	 * Applications may further add their own icon directories to this list,
	 * and users may extend or change the list (in application/desktop specific
	 * ways).In each of these directories themes are stored as subdirectories.
	 * A theme can be spread across several base directories by having
	 * subdirectories of the same name. This way users can extend and override
	 * system themes.
	 *
	 * In order to have a place for third party applications to install their
	 * icons there should always exist a theme called "hicolor" [1]. The data
	 * for the hicolor theme is available for download at:
	 * http://www.freedesktop.org/software/icon-theme/. Implementations are
	 * required to look in the "hicolor" theme if an icon was not found in the
	 * current theme.*/

	// PATH
	if (_iconName.startsWith('/'))
		return QIcon(_iconName);

	// STD WAY
	if (QIcon::hasThemeIcon(_iconName))
		return QIcon::fromTheme(_iconName);

	// HICOLOR (Fallback)
	QString currTheme = QIcon::themeName();
	QIcon::setThemeName("Hicolor");

	if (QIcon::hasThemeIcon(_iconName)){
		QIcon retval = QIcon::fromTheme(_iconName);
		QIcon::setThemeName(currTheme);
		return retval;
	}

	// Try again without extension if there is one
	if (_iconName.contains('.')){
		QString s =  _iconName.section('.',0,-2);
		if (QIcon::hasThemeIcon(s))	{
			QIcon retval = QIcon::fromTheme(s);
			QIcon::setThemeName(currTheme);
			return retval;
		}
	}

	// PIXMAPS
	QDirIterator it("/usr/share/pixmaps", QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QFileInfo fi = it.fileInfo();
		if (fi.isFile() && fi.baseName().startsWith(_iconName))
			return QIcon(fi.canonicalFilePath());
	}

	//UNKNOWN
	return QIcon::fromTheme("unknown");
}
