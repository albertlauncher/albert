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

#include <functional>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

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
	if (_widget == nullptr)
		_widget = new AppIndexWidget(this);
	return _widget;
}

/**************************************************************************/
void AppIndex::initialize()
{
	// Initially index std paths
	_paths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).toSet();

	// Selfexplanatory
	buildIndex();
}

/**************************************************************************/
void AppIndex::buildIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();

	qDebug() << "[ApplicationIndex]\tLooking in: " << _paths;

	// Define a lambda for recursion
	// This lambdsa makes no sanity checks since the directories in the recursion are always
	// valid an would simply produce overhead -> check for sanity before use
	std::function<void(const QFileInfo& fi)> rec_dirsearch = [&] (const QFileInfo& fi)
	{
		if (fi.isDir())
		{
			QDir d(fi.absoluteFilePath());
			d.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);

			// go recursive into subdirs
			QFileInfoList list = d.entryInfoList();
			for ( QFileInfo &dfi : list)
				rec_dirsearch(dfi);
		}
		else
		{
			// Check extension
			if (fi.suffix() != QString::fromLocal8Bit("desktop"))
				return;

			// Read the entries in the desktopfile
			QMap<QString, QString> desktopfile;
			QFile file(fi.absoluteFilePath());
			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
				return;
			QTextStream in(&file);
			QString line = in.readLine();
			while (!line.isNull()) {
				desktopfile[line.section('=', 0, 0)] = line.section('=', 1);
				line = in.readLine();
			}

			// Check if this shall be displayed
			if (desktopfile["NoDisplay"].compare("true", Qt::CaseInsensitive) == 0)
				return;

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
	};

	// Finally do this recursion for all paths
	for ( const QString &p : _paths) {
		QFileInfo fi(p);
		if (fi.exists())
			rec_dirsearch(fi);
	}

	std::sort(_index.begin(), _index.end(), Service::Item::CaseInsensitiveCompare());

	qDebug() << "[ApplicationIndex]\tFound " << _index.size() << " apps.";
}

/**************************************************************************/
QDataStream &AppIndex::serialize(QDataStream &out) const
{
	out << _paths
		<< _index.size()
		<< static_cast<int>(searchType());
	for (Service::Item *it : _index)
		static_cast<AppIndex::Item*>(it)->serialize(out);
	return out;
}

/**************************************************************************/
QDataStream &AppIndex::deserialize(QDataStream &in)
{
	int size, T;
	in >> _paths
			>> size
			>> T;
	AppIndex::Item *it;
	for (int i = 0; i < size; ++i) {
		it = new AppIndex::Item;
		it->deserialize(in);
		_index.push_back(it);
	}
	setSearchType(static_cast<IndexService::SearchType>(T));
	setSearchType(static_cast<IndexService::SearchType>(T));
	qDebug() << "[ApplicationIndex]\tLoaded " << _index.size() << " apps.";
	return in;
}
