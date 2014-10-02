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

#include "fileindex.h"
#include "fileitem.h"
#include <functional>
#include <chrono>
#include <algorithm>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QFile>


/**************************************************************************/
FileIndex::FileIndex()
{
	buildIndex();
	qDebug() << "[FileIndex]\t\tIndexing done. Found " << _index.size() << " files.";
	std::sort(_index.begin(), _index.end(), Index::CaseInsensitiveCompare());

	for (auto *i : _index)
		qDebug() << i->title();

	setSearchType(Index::SearchType::WordMatch);
}

/**************************************************************************/
FileIndex::~FileIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();
}

/**************************************************************************/
void FileIndex::buildIndex()
{
	QSettings conf;
	QStringList paths = conf.value(QString::fromLocal8Bit("file_index_paths")).toStringList();
	qDebug() << "[FileIndex]\t\tLooking in: " << paths;

	// Define a lambda for recursion
	// This lambdsa makes no sanity checks since the directories in the recursion are always
	// valid an would simply produce overhead -> check for sanity before use
	std::function<void(const QFileInfo& p)> rec_dirsearch = [&] (const QFileInfo& fi)
	{
		Item *i = new Item;
		i->_name = fi.fileName();
		i->_path = fi.absolutePath();
		_index.push_back(i);

		if (fi.isDir())
		{
			QDir d(fi.absoluteFilePath());
			d.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);
			if (conf.value(QString::fromLocal8Bit("index_hidden_files"), false).toBool())
				d.setFilter(d.filter() | QDir::Hidden);

			// go recursive into subdirs
			QFileInfoList list = d.entryInfoList();
			for ( QFileInfo &dfi : list)
				rec_dirsearch(dfi);
		}
	};

	// Finally do this recursion for all paths
	for ( QString &p : paths) {
		QFileInfo fi(p);
		if (fi.exists())
			rec_dirsearch(fi);
	}

	std::sort(_index.begin(), _index.end(), CaseInsensitiveCompare());
}

/**************************************************************************/
void FileIndex::save(const QString& p) const
{
	// If there is a serialized index use it
	QFile file(p);
	if (file.open(QIODevice::ReadWrite| QIODevice::Text))
	{
		qDebug() << "[FileIndex]\t\tSerializing to" << p;
		QDataStream stream( &file );
		stream << _index.size();
		for (Index::Item *i : _index)
			stream << *static_cast<FileIndex::Item*>(i);
		file.close();
		return;
	}
}

/**************************************************************************/
void FileIndex::load(const QString& p)
{
	// Exit if the file does not exist
	QFile file(p);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "[FileIndex]\tDeserializing from" << p;

		// Open the stream an load all data
		QDataStream stream( &file );
		int size;
		stream >> size;
		FileIndex::Item* tmpItem;
		for (int i = 0; i < size; ++i) {
			tmpItem = new FileIndex::Item;
			stream >> *tmpItem;
			_index.push_back(tmpItem);
		}
		file.close();
	}
}
