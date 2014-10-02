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

#include "applicationindex.h"
#include "applicationitem.h"
#include <functional>
//#include "websearch/websearch.h"
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QString>
#include <QStandardPaths>
#include <algorithm>
#include <QDataStream>
#include <QMetaType>


/**************************************************************************/
ApplicationIndex::ApplicationIndex()
{
	buildIndex();
	qDebug() << "[ApplicationIndex]\tIndexing done. Found " << _index.size() << " apps.";
	std::sort(_index.begin(), _index.end(), Index::CaseInsensitiveCompare());

//	for(auto *i : _index)
//		qDebug() << i->title();

	setSearchType(Index::SearchType::WordMatch);
}

/**************************************************************************/
ApplicationIndex::~ApplicationIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();
}

/**************************************************************************/
void ApplicationIndex::buildIndex()
{
	QStringList paths = QSettings().value(QString::fromLocal8Bit("app_index_paths")).toStringList();
	qDebug() << "[ApplicationIndex]\tLooking in: " << paths;

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

			// Strip every desktop file params
			QString exec = desktopfile["Exec"];
			exec.remove(QRegExp("%."));

			Item *i = new Item;
			i->_name     = desktopfile["Name"];
			i->_info     = (desktopfile["Comment"].isEmpty())?desktopfile["GenericName"]:desktopfile["Comment"];
			i->_iconName = desktopfile["Icon"];
			i->_exec     = exec;
			i->_term     = term;
			_index.push_back(i);
		}
	};

	// Finally do this recursion for all paths
	for ( QString &p : paths) {
		QFileInfo fi(p);
		if (fi.exists())
			rec_dirsearch(fi);
	}
}

/**************************************************************************/
void ApplicationIndex::load(const QString &f)
{
	QFile file(f);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "[ApplicationIndex]\tDeserializing from" << f;
		QDataStream stream( &file );
		int size;
		stream >> size;
		ApplicationIndex::Item* tmpItem;
		for (int i = 0; i < size; ++i) {
			tmpItem = new ApplicationIndex::Item;
			stream >> *tmpItem;
			_index.push_back(tmpItem);
		}
		file.close();
		return;
	}
}

/**************************************************************************/
void ApplicationIndex::save(const QString&f) const
{
	qDebug() << "[ApplicationIndex]\tSerializing to" << f;
	// If there is a serialized index use it
	QFile file(f);
	if (file.open(QIODevice::ReadWrite| QIODevice::Text))
	{
		QDataStream stream( &file );
		stream << _index.size();
		for (Service::Item *i : _index)
			stream << *static_cast<ApplicationIndex::Item*>(i);
		file.close();
		return;
	}
}
