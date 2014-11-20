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

#include "appindex.h"
#include "appitem.h"
#include "appindexwidget.h"

#include "wordmatchsearch.h"
#include "fuzzysearch.h"

#include <QDebug>
#include <QDirIterator>
#include <QString>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include "windows.h"
#endif
/**************************************************************************/
AppIndex::AppIndex()
{
	// Rebuild index if watcher signaled a change
	connect(&_watcher, &QFileSystemWatcher::directoryChanged, [&](){
		buildIndex();
		qDebug() << "[ApplicationIndex]\tIndex rebuilt";
	});
}

/**************************************************************************/
AppIndex::~AppIndex()
{
}

/**************************************************************************/
QWidget *AppIndex::widget()
{
	return new AppIndexWidget(this);
}

/**************************************************************************/
void AppIndex::initialize()
{
	restorePaths();
	buildIndex();
}

/**************************************************************************/
QStringList AppIndex::paths() const
{
	return _watcher.directories();
}

/**************************************************************************/
bool AppIndex::addPath(const QString & s)
{
	bool failed = _watcher.addPath(s);
	if (!failed)
		buildIndex();
	return failed;
}

/**************************************************************************/
bool   AppIndex::removePath(const QString & s)
{
	bool failed = _watcher.removePath(s);
	if (!failed)
		buildIndex();
	return failed;
}

/**************************************************************************/
void AppIndex::restorePaths()
{
	_watcher.removePaths(_watcher.directories());
	_watcher.addPaths(QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation));
	buildIndex();
}

/**************************************************************************/
void AppIndex::saveSettings(QSettings &s) const
{
	// Save settings
	s.beginGroup("AppIndex");
	s.setValue("Paths", _watcher.directories());
	s.setValue("Fuzzy", dynamic_cast<FuzzySearch*>(_search) != nullptr);
	s.endGroup();
}

/**************************************************************************/
void AppIndex::loadSettings(QSettings &s)
{
	// Load settings
	s.beginGroup("AppIndex");
	_watcher.addPaths(s.value("Paths", QStandardPaths::standardLocations(
						 QStandardPaths::ApplicationsLocation)).toStringList());
	if(s.value("Fuzzy",false).toBool())
		setSearch(new FuzzySearch());
	else
		setSearch(new WordMatchSearch());
	s.endGroup();
}

/**************************************************************************/
void AppIndex::serilizeData(QDataStream &out) const
{
	// Serialize data
	out << _index.size();
	for (Service::Item *it : _index)
		static_cast<AppIndex::Item*>(it)->serialize(out);
}

/**************************************************************************/
void AppIndex::deserilizeData(QDataStream &in)
{
	// Deserialize the index
	int size;
	in >> size;
	AppIndex::Item *it;
	emit beginBuildIndex();
	for (int i = 0; i < size; ++i) {
		it = new AppIndex::Item;
		it->deserialize(in);
		it->_icon = getIcon(it->_iconName);
		_index.push_back(it);
	}
	emit endBuildIndex();
	qDebug() << "[ApplicationIndex]\tLoaded " << _index.size() << " apps.";
}

/**************************************************************************/
void AppIndex::query(const QString &req, QVector<Service::Item *> *res) const
{
	_search->query(req, res);
}

/**************************************************************************/
void AppIndex::queryFallback(const QString &, QVector<Service::Item *> *) const
{

}

/**************************************************************************/
void AppIndex::buildIndex()
{
	emit beginBuildIndex();

	for(Service::Item *i : _index)
		delete i;
	_index.clear();

	qDebug() << "[ApplicationIndex]\tLooking in: " << _watcher.directories();

#ifdef Q_OS_LINUX
	for ( const QString &p : _watcher.directories()) {
		QDirIterator it(p, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			// Check extension
			if (fi.suffix() != QString::fromLocal8Bit("desktop"))
				continue;

			// TYPES http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html

			// Read the entries in the desktopfile
			QMap<QString, QString> desktopfile;
			QFile file(fi.absoluteFilePath());
			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
				continue;
			QTextStream in(&file);
			QString line;

			// Skip everything until [Desktop Entry]
			do {
				line = in.readLine();
			} while (line.trimmed().compare("[Desktop Entry]") != 0);


			// Read everything until end or next section
			line = in.readLine();
			while (!line.isNull() && ! line.startsWith('[')){
				desktopfile[line.section('=', 0, 0)] = line.section('=', 1);
				line = in.readLine();
			}

			// Check if this shall be displayed
			if (desktopfile["NoDisplay"].compare("true", Qt::CaseInsensitive) == 0)
				continue;

			// Check if this shall be runned in terminal
			bool term = (desktopfile["Terminal"].compare("true", Qt::CaseInsensitive) == 0);

			// Check if there exists a lcoalized name
			QString localeShortcut = QLocale().name();
			localeShortcut.truncate(2);
			QString name = desktopfile.value(QString("Name[%1]").arg(localeShortcut), desktopfile["Name"]);

			//  http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html
			QString exec = desktopfile["Exec"];
			exec.replace("%c", name);

			// Remove other placeholders
			exec.remove(QRegExp("%."));

			Item *i = new Item;
			i->_name = name;
			i->_info = (desktopfile["Comment"].isEmpty())?desktopfile["GenericName"]:desktopfile["Comment"];
			i->_iconName = desktopfile["Icon"];
			i->_icon = getIcon(i->_iconName);
			i->_exec = exec;
			i->_term = term;
			_index.push_back(i);
		}
	}
#endif
#ifdef Q_OS_WIN
// TODO QTBUG-40565
	//	for ( const QString &p : _paths) {
		QDirIterator it(
					"C:/Documents and Settings/All Users/Start Menu/Programs",
				   QDir::Files|QDir::NoDotAndDotDot,
					QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			if (fi.isExecutable())
			{
				Item *i = new Item;
				qDebug()<< fi.baseName();
				i->_name     = fi.baseName();
				if (fi.isSymLink())
					fi.setFile(fi.symLinkTarget());
				qDebug()<< fi.fileName();
				qDebug()<< fi.canonicalFilePath();
				i->_info     = fi.canonicalFilePath();
				i->_icon     = getIcon(fi.canonicalFilePath());
				i->_exec     = QString("\"%1\"").arg(fi.canonicalFilePath());
				i->_term     = false;
				_index.push_back(i);
			}

		}
//    }
//	HKEY hUninstKey = NULL;
//	HKEY hAppKey = NULL;
//	WCHAR sAppKeyName[1024];
//	WCHAR sSubKey[1024];
//	WCHAR sDisplayName[1024];
//	WCHAR *sRoot = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
//	long lResult = ERROR_SUCCESS;
//	DWORD dwType = KEY_ALL_ACCESS;
//	DWORD dwBufferSize = 0;

//	//Open the "Uninstall" key.
//	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, sRoot, 0, KEY_READ, &hUninstKey) != ERROR_SUCCESS)
//	{
//		return;
//	}

//	for(DWORD dwIndex = 0; lResult == ERROR_SUCCESS; dwIndex++)
//	{
//		//Enumerate all sub keys...
//		dwBufferSize = sizeof(sAppKeyName);
//		if((lResult = RegEnumKeyEx(hUninstKey, dwIndex, sAppKeyName,
//			&dwBufferSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
//		{
//			//Open the sub key.
//			wsprintf(sSubKey, L"%s\\%s", sRoot, sAppKeyName);
//			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, 0, KEY_READ, &hAppKey) != ERROR_SUCCESS) {
//				RegCloseKey(hAppKey);
//				RegCloseKey(hUninstKey);
//				return;
//			}

//			//Get the display name value from the application's sub key.
//			dwBufferSize = sizeof(sDisplayName);
//			if(RegQueryValueEx(hAppKey, L"DisplayName", NULL,
//				&dwType, (unsigned char*)sDisplayName, &dwBufferSize) == ERROR_SUCCESS) {
//				qDebug() << QString::fromWCharArray(sAppKeyName);
//				qDebug() << QString::fromWCharArray(sSubKey);
//				qDebug() << QString::fromWCharArray(sDisplayName);


//				Item *i = new Item;
//				i->_name     = QString::fromWCharArray(sDisplayName);
//				i->_info     = "";
//				i->_iconName = "";
//				i->_exec     = "";
//				i->_term     = false;
//				_index.push_back(i);
//			}

//			RegCloseKey(hAppKey);
//		}
//	}

//	RegCloseKey(hUninstKey);
#endif

	qDebug() << "[ApplicationIndex]\tFound " << _index.size() << " apps.";
	emit endBuildIndex();
}

#include "QFileIconProvider"
/**************************************************************************/
QIcon AppIndex::getIcon(QString iconName)
{
#ifdef Q_OS_LINUX
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
	if (iconName.startsWith('/'))
		return QIcon(iconName);

	// Strip suffix
	QString strippedIconName = iconName;
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

//	for (const QString & p : paths){
//		for (const QString & t : themes){
//			for (const QString & s : sizes){
//				qDebug() << QString("%1/icons/%2/%3/apps").arg(p,t,s);
//				QDirIterator it(QString("%1/icons/%2/%3/apps").arg(p,t,s), QDirIterator::FollowSymlinks);
//				while (it.hasNext()){
//					it.next();
//					QFileInfo fi = it.fileInfo();
//					if (fi.isFile() && fi.baseName() == strippedIconName)
//						return QIcon(fi.canonicalFilePath());
//				}
//			}
//		}
//	}

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
#endif
#ifdef Q_OS_WIN


	QFileIconProvider fip;
	qDebug() << iconName;
	return fip.icon(QFileInfo(iconName));
//	HICON ico = ExtractIconW(nullptr, this->_exec.toStdWString().c_str(), 0);

//	DestroyIcon(ico);
//	return QIcon(QPixmap::fromWinHICON(ico));

#endif
}

