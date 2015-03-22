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
#include "websearch/websearch.h"
#include <chrono>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

/**************************************************************************/
const QMimeDatabase FileIndex::Item::mimeDb;

/**************************************************************************/
void FileIndex::Item::action()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	QDesktopServices::openUrl(QUrl("file:///"+_fileInfo.canonicalFilePath()));
}

/**************************************************************************/
QString FileIndex::Item::actionText() const
{
	return QString("Open '%1' with default application.").arg(_fileInfo.fileName());
}

/**************************************************************************/
void FileIndex::Item::altAction()
{
	_lastAccess = std::chrono::system_clock::now().time_since_epoch().count();
	QDesktopServices::openUrl(QUrl("file:///"+_fileInfo.canonicalPath()));
}

/**************************************************************************/
QString FileIndex::Item::altActionText() const
{
	return QString("Open the folder containing '%1' in file browser.").arg(_fileInfo.fileName());
}

#include "QFileIconProvider"

/**************************************************************************/
QIcon FileIndex::Item::icon() const
{
#ifdef Q_OS_LINUX
	QString iconName = mimeDb.mimeTypeForFile(_fileInfo).iconName();
	if (QIcon::hasThemeIcon(iconName))
		return QIcon::fromTheme(iconName);
	return QIcon::fromTheme(QString::fromLocal8Bit("unknown"));

#endif

#ifdef Q_OS_WIN


	QFileIconProvider fip;
	return fip.icon(this->_fileInfo);
//	HICON ico = ExtractIconW(nullptr, this->_exec.toStdWString().c_str(), 0);

//	DestroyIcon(ico);
//	return QIcon(QPixmap::fromWinHICON(ico));

#endif
}

/**************************************************************************/
void FileIndex::Item::serialize(QDataStream &out) const
{
	out << _lastAccess << _fileInfo.canonicalFilePath();
}

/**************************************************************************/
void FileIndex::Item::deserialize(QDataStream &in)
{
	QString canonicalFilePath;
	in >> _lastAccess >> canonicalFilePath;
	_fileInfo.setFile(canonicalFilePath);
}
