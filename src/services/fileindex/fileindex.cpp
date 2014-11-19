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

#include "wordmatchsearch.h"
#include "fuzzysearch.h"

#include <algorithm>
#include <QDirIterator>
#include <QDebug>
#include <QStandardPaths>

/**************************************************************************/
FileIndex::FileIndex()
{}

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
	return new FileIndexWidget(this);
}

/**************************************************************************/
void FileIndex::initialize()
{
	buildIndex();
}

/**************************************************************************/
void FileIndex::restorePaths()
{
	_paths = QStringList({QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
						  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
						  QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
						  QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
						  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
						  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)});
}

/**************************************************************************/
void FileIndex::saveSettings(QSettings &s) const
{
	// Save settings
	s.beginGroup("FileIndex");
	s.setValue("Paths", _watcher.directories() <<_watcher.files());
	s.setValue("indexHiddenFiles", _indexHiddenFiles);
	s.setValue("Fuzzy", dynamic_cast<FuzzySearch*>(_search) != nullptr);
	s.endGroup();
}

/**************************************************************************/
void FileIndex::loadSettings(QSettings &s)
{
	// Load settings
	s.beginGroup("FileIndex");
	QStringList paths;
	paths << QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
		  << QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
	_paths = s.value("Paths", paths).toStringList();
	_indexHiddenFiles = s.value("indexHiddenFiles", false).toBool();
	if(s.value("Fuzzy",false).toBool())
		setSearch(new FuzzySearch());
	else
		setSearch(new WordMatchSearch());
	s.endGroup();
}

/**************************************************************************/
void FileIndex::serilizeData(QDataStream &out) const
{
	// Serialize data
	out	<< _index.size();
	for (Service::Item *it : _index)
		static_cast<FileIndex::Item*>(it)->serialize(out);
}

/**************************************************************************/
void FileIndex::deserilizeData(QDataStream &in)
{
	// Deserialize the index
	int size;
	in	>> size;
	FileIndex::Item *it;
	for (int i = 0; i < size; ++i) {
		it = new FileIndex::Item;
		it->deserialize(in);
		_index.push_back(it);
	}
	qDebug() << "[FileIndex]\tLoaded " << _index.size() << " files.";
}

/**************************************************************************/
void FileIndex::query(const QString &req, QVector<Service::Item *> *res) const
{
	_search->query(req, res);
}

/**************************************************************************/
void FileIndex::queryFallback(const QString &, QVector<Service::Item *> *) const
{

}

/**************************************************************************/
void FileIndex::buildIndex()
{
	emit beginBuildIndex();
	for(Service::Item *i : _index)
		delete i;
	_index.clear();

	qDebug() << "[FileIndex]\tLooking in: " << _paths;

	for (const QString &p : _paths)
		addPath(p);

	qDebug() << "[FileIndex]\tFound " << _index.size() << " files.";
	emit endBuildIndex();
}

/**************************************************************************/
bool FileIndex::addPath(const QString &path)
{
	QFileInfo fi(path);
	if (!fi.exists())
		return false;

	if (_paths.contains(path))
		return true;

	if (!_watcher.addPath(path))
		return false;

	_paths << path;

	if (fi.isDir()){
		QDirIterator it(fi.canonicalFilePath(), QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			if (it.fileInfo().isHidden() && !_indexHiddenFiles)
				continue;
			Item *i = new Item;
			i->_fileInfo = it.fileInfo();
			_index.push_back(i);
		}
	}
	else if (fi.isFile() || (!fi.isHidden() || _indexHiddenFiles) ) {
		Item *i = new Item;
		i->_fileInfo = fi;
		_index.push_back(i);
	}

	qDebug() << "[FileIndex]\tAdded" << path;
	return true;
}

/**************************************************************************/
void FileIndex::removePath(const QString &path)
{
	_watcher.removePath(path);

	QFileInfo fi(path);
	if (fi.isFile() || (!fi.isHidden() || _indexHiddenFiles) )
		for (QList<Service::Item*>::iterator it = _index.begin(); it != _index.end();)
			((*it)->infoText().startsWith(path))?it = _index.erase(it):++it;

	qDebug() << "[FileIndex]\tRemoved" << path;
}

/**************************************************************************/
void FileIndex::HAPPENING(const QString &path)
{
	qDebug() << "WOW:" << path;

}
