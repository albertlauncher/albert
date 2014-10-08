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

#include <functional>
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStandardPaths>

/**************************************************************************/
BookmarkIndex::~BookmarkIndex()
{
	for(Service::Item *i : _index)
		delete i;
	_index.clear();
}

/**************************************************************************/
QWidget *BookmarkIndex::widget()
{
	if (_widget == nullptr)
		_widget = new BookmarkIndexWidget(this);
	return _widget;
}

/**************************************************************************/
void BookmarkIndex::initialize()
{
	buildIndex();
	std::sort(_index.begin(), _index.end(), Service::Item::CaseInsensitiveCompare());
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

	QFile f(bookmarkPath);
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning() << "[BookmarkIndex]\tCould not open" << bookmarkPath;
		return;
	}

	QJsonObject json = QJsonDocument::fromJson(f.readAll()).object();
	QJsonObject roots = json.value("roots").toObject();
	for (const QJsonValue &i : roots)
		if (i.isObject())
			rec_bmsearch(i.toObject());

	qDebug() << "[BookmarkIndex]\tFound " << _index.size() << " bookmarks.";
}

/**************************************************************************/
QDataStream &BookmarkIndex::serialize(QDataStream &out) const
{
	out << _index.size();
	for (Service::Item *it : _index)
		static_cast<BookmarkIndex::Item*>(it)->serialize(out);
	out << static_cast<int>(searchType());
	return out;
}

/**************************************************************************/
QDataStream &BookmarkIndex::deserialize(QDataStream &in)
{
	int size;
	in >> size;
	BookmarkIndex::Item *it;
	for (int i = 0; i < size; ++i) {
		it = new BookmarkIndex::Item;
		it->deserialize(in);
		_index.push_back(it);
	}
	int T;
	in >> T;
	setSearchType(static_cast<IndexService::SearchType>(T));
	qDebug() << "[BookmarkIndex]\tLoaded " << _index.size() << " bookmarks.";
	return in;
}
