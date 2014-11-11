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

#include <QDebug>
#include <QDirIterator>
#include <QString>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include "windows.h"
#endif
/**************************************************************************/
AppIndex::~AppIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();
}

/**************************************************************************/
QWidget *AppIndex::widget()
{
	return new AppIndexWidget(this);
}

/**************************************************************************/
void AppIndex::initialize()
{
	restoreDefaults();
	buildIndex();
}

/**************************************************************************/
void AppIndex::restoreDefaults()
{
	setSearchType(SearchType::WordMatch);
	_paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
}

/**************************************************************************/
void AppIndex::saveSettings(QSettings &s) const
{
	// Save settings
	s.beginGroup("AppIndex");
	s.setValue("Paths", _paths);
	s.setValue("SearchType", static_cast<int>(searchType()));
	s.endGroup();
}

/**************************************************************************/
void AppIndex::loadSettings(QSettings &s)
{
	// Load settings
	s.beginGroup("AppIndex");
	_paths = s.value("Paths", QStandardPaths::standardLocations(
						 QStandardPaths::ApplicationsLocation)).toStringList();
	setSearchType(static_cast<SearchType>(s.value("SearchType",1).toInt()));
	s.endGroup();
}

/**************************************************************************/
void AppIndex::serilizeData(QDataStream &out) const
{
	// Serialize data
	out << _index.size()
		<< static_cast<int>(searchType());
	for (Service::Item *it : _index)
		static_cast<AppIndex::Item*>(it)->serialize(out);
}

/**************************************************************************/
void AppIndex::deserilizeData(QDataStream &in)
{
	// Deserialize the index
	int size, T;
	in >> size >> T;
	AppIndex::Item *it;
	for (int i = 0; i < size; ++i) {
		it = new AppIndex::Item;
		it->deserialize(in);
		_index.push_back(it);
	}
	setSearchType(static_cast<IndexService::SearchType>(T));
	qDebug() << "[ApplicationIndex]\tLoaded " << _index.size() << " apps.";
}

/**************************************************************************/
void AppIndex::queryFallback(const QString &, QVector<Service::Item *> *) const
{

}

/**************************************************************************/
void AppIndex::buildIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();

	qDebug() << "[ApplicationIndex]\tLooking in: " << _paths;

#ifdef Q_OS_LINUX
	for ( const QString &p : _paths) {
		QDirIterator it(p, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QFileInfo fi = it.fileInfo();
			// Check extension
			if (fi.suffix() != QString::fromLocal8Bit("desktop"))
				continue;

			// Read the entries in the desktopfile
			QMap<QString, QString> desktopfile;
			QFile file(fi.absoluteFilePath());
			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
				continue;
			QTextStream in(&file);
			QString line = in.readLine();
			while (!line.isNull()) {
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

			// Replace placeholders
			/*
			 * Code	Description
			 * %f	A single file name, even if multiple files are selected.
			 * The system reading the desktop entry should recognize that the
			 * program in question cannot handle multiple file arguments, and
			 * it should should probably spawn and execute multiple copies of
			 * a program for each selected file if the program is not able to
			 * handle additional file arguments. If files are not on the local
			 * file system (i.e. are on HTTP or FTP locations), the files will
			 * be copied to the local file system and %f will be expanded to
			 * point at the temporary file. Used for programs that do not
			 * understand the URL syntax.
			 * %F	A list of files. Use for apps that can open several local
			 * files at once. Each file is passed as a separate argument to the
			 * executable program.
			 * %u	A single URL. Local files may either be passed as file: URLs
			 * or as file path.
			 * %U	A list of URLs. Each URL is passed as a separate argument to
			 * the executable program. Local files may either be passed as file:
			 * URLs or as file path.
			 * %d	Deprecated.
			 * %D	Deprecated.
			 * %n	Deprecated.
			 * %N	Deprecated.
			 * %i	The Icon key of the desktop entry expanded as two arguments,
			 * first --icon and then the value of the Icon key. Should not
			 * expand to any arguments if the Icon key is empty or missing.
			 * %c	The translated name of the application as listed in the
			 * appropriate Name key in the desktop entry.
			 * %k	The location of the desktop file as either a URI (if for
			 * example gotten from the vfolder system) or a local filename or
			 * empty if no location is known.
			 * %v	Deprecated.
			 * %m	Deprecated.
			*/
			QString exec = desktopfile["Exec"];
			exec.replace("%c", name);

			// Remove other placeholders
			exec.remove(QRegExp("%."));

			Item *i = new Item;
			i->_name     = name;
			i->_info     = (desktopfile["Comment"].isEmpty())?desktopfile["GenericName"]:desktopfile["Comment"];
			i->_iconName = desktopfile["Icon"];
			i->_exec     = exec;
			i->_term     = term;
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
				i->_iconName = "";
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
	prepareSearch();
}


