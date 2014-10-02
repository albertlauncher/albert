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

#include <functional>
#include <QSettings>
//#include <algorithm>
//#include <QDataStream>
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
	buildIndex();
	qDebug() << "[BookmarkIndex]\tIndexing done. Found " << _index.size() << " bookmarks.";
	std::sort(_index.begin(), _index.end(), Index::CaseInsensitiveCompare());

//	for(auto *i : _index)
//		qDebug() << i->title();

	setSearchType(Index::SearchType::WordMatch);

}

/**************************************************************************/
BookmarkIndex::~BookmarkIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();
}

/**************************************************************************/
void BookmarkIndex::buildIndex()
{
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

	// Finally do this recursion for all paths
	QString bookmarkPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/"
			+ QSettings().value(QString::fromLocal8Bit("chromium_bookmark_path"),
								QString::fromLocal8Bit(".config/chromium/Default/Bookmarks")).toString();

	qDebug() << "[BookmarkIndex]\tParsing" << bookmarkPath;

	QFile loadFile(bookmarkPath);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning() << "[BookmarkIndex]\tCould not open" << bookmarkPath;
		return;
	}

	QJsonObject json = QJsonDocument::fromJson(loadFile.readAll()).object();
	QJsonObject roots = json.value("roots").toObject();
	for (const QJsonValue &i : roots)
		if (i.isObject())
			rec_bmsearch(i.toObject());

	std::sort(_index.begin(), _index.end(), CaseInsensitiveCompare());
}

/**************************************************************************/
void BookmarkIndex::save(const QString& f) const
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
//	qDebug() << "[BookmarkIndex]\tSerializing to" << f; TODO
//	// If there is a serialized index use it
//	QFile file(f);
//	if (file.open(QIODevice::ReadWrite| QIODevice::Text))
//	{
//		QDataStream stream( &file );
//		stream << _index.size();
//		for (Index::Item *i : _index)
//			stream << *static_cast<BookmarkIndex::Item*>(i);
//		file.close();
//		return;
//	}
}

/**************************************************************************/
void BookmarkIndex::load(const QString& f)
{
	//TODO
	qDebug() << "NOT IMPLEMENTED!";
	exit(1);
//	// If there is a serialized index use it TODO
//	QFile file(_indexFile);
//	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
//	{
//		qDebug() << "[FileIndex]\tDeserializing from" << _indexFile;
//		QDataStream stream( &file );
//		int size;
//		stream >> size;
//		BookmarkIndex::Item* tmpItem;
//		for (int i = 0; i < size; ++i) {
//			tmpItem = new BookmarkIndex::Item;
//			stream >> *tmpItem;
//			_index.push_back(tmpItem);
//		}
//		file.close();
//		return;
//	}
}

