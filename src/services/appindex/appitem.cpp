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
#include <QStandardPaths>

QString AppIndex::Item::_termEmu;
QString AppIndex::Item::_suHelper;

/**************************************************************************/
void AppIndex::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	if (_term)
		QProcess::startDetached(_termEmu + " -e " + _exec);
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
		QProcess::startDetached(_suHelper + " " + _termEmu + " -e " + _exec);
	else
		QProcess::startDetached(_suHelper + " " + _exec);
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

	// Strip suffix
	QString strippedIconName = _iconName;
	if (strippedIconName.contains('.'))
		strippedIconName =  strippedIconName.section('.',0,-2);

	if (QIcon::hasThemeIcon(strippedIconName)) // HORRIBLY BUGGY QTBUG-42239 CHNAGE WITH Qt5.4
		return QIcon::fromTheme(strippedIconName);

	// Implementation for desktop specs
	QStringList paths, themes, sizes;
	paths << QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
	themes << QIcon::themeName() << "hicolor";
	sizes << "scalable" << "512x512" << "384x384" << "256x256" << "192x192"
		  << "128x128" << "96x96" << "72x72" << "64x64" << "48x48" << "42x42"
		  << "36x36" << "32x32" << "24x24" << "22x22" << "16x16" << "8x8";

	for (const QString & p : paths){
		for (const QString & t : themes){
			for (const QString & s : sizes){
				qDebug() << QString("%1/icons/%2/%3/apps").arg(p,t,s);
				QDirIterator it(QString("%1/icons/%2/%3/apps").arg(p,t,s), QDirIterator::FollowSymlinks);
				while (it.hasNext()){
					it.next();
					QFileInfo fi = it.fileInfo();
					if (fi.isFile() && fi.baseName() == strippedIconName)
						return QIcon(fi.canonicalFilePath());
				}
			}
		}
	}

	// PIXMAPS
	QDirIterator it("/usr/share/pixmaps", QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QFileInfo fi = it.fileInfo();
		if (fi.isFile() && fi.baseName() == strippedIconName)
			return QIcon(fi.canonicalFilePath());
	}

	//UNKNOWN
	return QIcon::fromTheme("unknown");
}
