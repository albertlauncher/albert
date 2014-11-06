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
#include "fileindexwidget.h"
#include "globals.h"

#include <algorithm>
#include <QDirIterator>
#include <QDebug>
#include <QStandardPaths>

/**************************************************************************/
FileIndex::~FileIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();
}

/**************************************************************************/
QWidget *FileIndex::widget()
{
	if (_widget == nullptr)
		_widget = new FileIndexWidget(this);
	return _widget;
}

/**************************************************************************/
void FileIndex::initialize()
{
	restoreDefaults();
	buildIndex();
}

/**************************************************************************/
void FileIndex::restoreDefaults()
{
	setSearchType(SearchType::WordMatch);

	gSettings->beginGroup("FileIndex");
	QStringList paths;
	paths << QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
	_paths = paths; //TODO
	gSettings->setValue("paths", paths);
	gSettings->endGroup();

}

/**************************************************************************/
void FileIndex::buildIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();

	qDebug() << "[FileIndex]\tLooking in: " << _paths;

	for ( const QString &p : _paths) {
		QDirIterator it(p, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			if (it.fileInfo().isHidden()
				&& !gSettings->value("indexHiddenFiles", false).toBool())
				continue;
			Item *i = new Item;
			i->_fileInfo = it.fileInfo();
			_index.push_back(i);
		}
	}

	std::sort(_index.begin(), _index.end(), Service::Item::CaseInsensitiveCompare());

	qDebug() << "[FileIndex]\tFound " << _index.size() << " files.";
}

/**************************************************************************/
QDataStream &FileIndex::serialize(QDataStream &out) const
{
	out << _paths
		<< _index.size()
		<< static_cast<int>(searchType());
	for (Service::Item *it : _index)
		static_cast<FileIndex::Item*>(it)->serialize(out);
	return out;
}

/**************************************************************************/
QDataStream &FileIndex::deserialize(QDataStream &in)
{
	int size, T;
	in >> _paths
			>> size
			>> T;
	FileIndex::Item *it;
	for (int i = 0; i < size; ++i) {
		it = new FileIndex::Item;
		it->deserialize(in);
		_index.push_back(it);
	}
	setSearchType(static_cast<IndexService::SearchType>(T));
	qDebug() << "[FileIndex]\tLoaded " << _index.size() << " files.";
	return in;
}


