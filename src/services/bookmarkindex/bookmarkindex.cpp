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

#include "bookmarkindex.h"
#include "bookmarkitem.h"
#include "bookmarkindexwidget.h"

#include "wordmatchsearch.h"
#include "fuzzysearch.h"

#include <functional>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStandardPaths>

/**************************************************************************/
BookmarkIndex::BookmarkIndex()
{
	// Rebuild index if bookmarkfile changed
	connect(&_watcher, &QFileSystemWatcher::fileChanged, [&](const QString & p){
		// QFileSystemWatcher stops monitoring files once they have been
		// renamed or removed from disk, hence rewatch.
		_watcher.addPath(p);
		buildIndex();
		qDebug() << "[BookmarkIndex]\tIndex rebuilt";
	});
}

/**************************************************************************/
BookmarkIndex::~BookmarkIndex()
{
}

/**************************************************************************/
QWidget *BookmarkIndex::widget()
{
	return new BookmarkIndexWidget(this);
}

/**************************************************************************/
void BookmarkIndex::initialize()
{
	restorePath();
	buildIndex();
}

/**************************************************************************/
QString BookmarkIndex::path() const
{
	return (_watcher.files().isEmpty()) ? "" : _watcher.files()[0];
}

/**************************************************************************/
bool BookmarkIndex::setPath(const QString &s)
{
	QFileInfo fi(s);
	// Only let files in
	if (!(fi.exists() && fi.isFile()))
		return false;

	// Remove old path
	unsetPath();
	return _watcher.addPath(s);
}

/**************************************************************************/
void BookmarkIndex::unsetPath()
{
	_watcher.removePaths(_watcher.files());
}

/**************************************************************************/
void BookmarkIndex::restorePath()
{
	if (!setPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
			+ "/chromium/Default/Bookmarks"))
		qWarning("[BookmarkIndex]Unable to restore bookmark path.");
}

/**************************************************************************/
void BookmarkIndex::saveSettings(QSettings &s) const
{
	// Save settings
	s.beginGroup("BookmarkIndex");
	s.setValue("Path", path());
	s.setValue("Fuzzy", dynamic_cast<FuzzySearch*>(_search) != nullptr);
	s.endGroup();
}

/**************************************************************************/
void BookmarkIndex::loadSettings(QSettings &s)
{
	// Load settings
	s.beginGroup("BookmarkIndex");
	if (s.contains("Path")){
		if (!setPath(s.value("Path").toString()))
			qWarning("Could not set path %s", s.value("Path").toString().toStdString().c_str());
	}
	else
		restorePath();
	if(s.value("Fuzzy",false).toBool())
		setSearch(new FuzzySearch());
	else
		setSearch(new WordMatchSearch());
	s.endGroup();
}

/**************************************************************************/
void BookmarkIndex::serilizeData(QDataStream &out) const
{
	// Serialize data
	out << _index.size();
	for (Service::Item *it : _index)
		static_cast<BookmarkIndex::Item*>(it)->serialize(out);
}

/**************************************************************************/
void BookmarkIndex::deserilizeData(QDataStream &in)
{
	// Deserialize the index
	int size;
	in >> size;
	BookmarkIndex::Item *it;
	for (int i = 0; i < size; ++i) {
		it = new BookmarkIndex::Item;
		it->deserialize(in);
		_index.push_back(it);
	}
	qDebug() << "[BookmarkIndex]\tLoaded " << _index.size() << " bookmarks.";
}

/**************************************************************************/
void BookmarkIndex::query(const QString &req, QVector<Service::Item *> *res) const
{
	_search->query(req, res);
}

/**************************************************************************/
void BookmarkIndex::queryFallback(const QString &, QVector<Service::Item *> *) const
{

}

/**************************************************************************/
void BookmarkIndex::buildIndex()
{
	emit beginBuildIndex();
	for(Service::Item *i : _index)
		delete i;
	_index.clear();

	// Define a lambda for recursion
	std::function<void(const QJsonObject &json)> rec_bmsearch = [&] (const QJsonObject &json)
	{
		QJsonValue type = json["type"];
		if (type == QJsonValue::Undefined)
			return;
		if (type.toString() == "folder"){
			QJsonArray jarr = json["children"].toArray();
			for (const QJsonValue &i : jarr)
				rec_bmsearch(i.toObject());
		}
		if (type.toString() == "url") {
			Item *i = new Item;
			i->_title = json["name"].toString();
			i->_url   = json["url"].toString();
			_index.append(i);
		}
	};

	qDebug() << "[BookmarkIndex]\tParsing" << path();

	QFile f(path());
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning() << "[BookmarkIndex]\tCould not open" << path();
		return;
	}

	QJsonObject json = QJsonDocument::fromJson(f.readAll()).object();
	QJsonObject roots = json.value("roots").toObject();
	for (const QJsonValue &i : roots)
		if (i.isObject())
			rec_bmsearch(i.toObject());

	qDebug() << "[BookmarkIndex]\tFound " << _index.size() << " bookmarks.";
	emit endBuildIndex();
}

