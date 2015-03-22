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

#include "websearch.h"
#include "websearchitem.h"
#include "websearchwidget.h"

#include <QDesktopServices>
#include <QStandardPaths>
#include <QUrl>
#include <QFile>
#include <QDebug>

/**************************************************************************/
void WebSearch::initialize()
{
	restoreDefaults();
}

/**************************************************************************/
void WebSearch::restoreDefaults()
{

	/* Init std searches */
	for(Service::Item *i : _searchEngines)
		delete i;
	_searchEngines.clear();

	// Google
	Item *i = new Item;
	i->_lastAccess = 1;
	i->_name       = "Google";
	i->_url        = "https://www.google.de/#q=%s";
	i->_shortcut   = "gg";
	// this should give back the system data path + /icons/icon.svg
	for (QString s : QStandardPaths::standardLocations(QStandardPaths::DataLocation)) {
		QFile f(s.append("/icons/google.svg"));
		if (f.exists())
			i->_iconPath = s;
	}
	_searchEngines.push_back(i);

	// Youtube
	i = new Item;
	i->_lastAccess = 1;
	i->_name       = "Youtube";
	i->_url        = "https://www.youtube.com/results?search_query=%s";
	i->_shortcut   = "yt";
	for (QString s : QStandardPaths::standardLocations(QStandardPaths::DataLocation)) {
		QFile f(s.append("/icons/youtube.svg"));
		if (f.exists())
			i->_iconPath = s;
	}
	_searchEngines.push_back(i);

	// Amazon
	i = new Item;
	i->_lastAccess = 1;
	i->_name       = "Amazon";
	i->_url        = "http://www.amazon.de/s/?field-keywords=%s";
	i->_shortcut   = "ama";
	for (QString s : QStandardPaths::standardLocations(QStandardPaths::DataLocation)) {
		QFile f(s.append("/icons/amazon.svg"));
		if (f.exists())
			i->_iconPath = s;
	}
	_searchEngines.push_back(i);

	// Ebay
	i = new Item;
	i->_lastAccess = 1;
	i->_name       = "Ebay";
	i->_url        = "http://www.ebay.de/sch/i.html?_nkw=%s";
	i->_shortcut   = "eb";
	for (QString s : QStandardPaths::standardLocations(QStandardPaths::DataLocation)) {
		QFile f(s.append("/icons/ebay.svg"));
		if (f.exists())
			i->_iconPath = s;
	}
	_searchEngines.push_back(i);
}

/**************************************************************************/
void WebSearch::saveSettings(QSettings &s) const
{
	// Save settings
	s.beginGroup("WebSearch");
	s.endGroup();
}

/**************************************************************************/
void WebSearch::loadSettings(QSettings &s)
{
	// Load settings
	s.beginGroup("WebSearch");
	s.endGroup();
}

/**************************************************************************/
void WebSearch::serilizeData(QDataStream &out) const
{
	// Serialize data
	out << _searchEngines.size();
	for (WebSearch::Item* it : _searchEngines)
		 it->serialize(out);
}

/**************************************************************************/
void WebSearch::deserilizeData(QDataStream &in)
{
	// Deserialize the index
	int size;
	in >> size;
	for (int i = 0; i < size; ++i) {
		WebSearch::Item *it = new WebSearch::Item;
		it->deserialize(in);
		_searchEngines.push_back(it);
	}
}

/**************************************************************************/
QWidget *WebSearch::widget()
{
	return new WebSearchWidget(this);
}

/**************************************************************************/
void WebSearch::query(const QString &req, QVector<Service::Item *> *res) const
{
	QString firstWord = req.section(' ',0,0);
	for (Item *w : _searchEngines)
		if ((firstWord.compare(w->_name, Qt::CaseInsensitive)==0) || ((firstWord.compare(w->_shortcut, Qt::CaseInsensitive)==0)))
		{
			w->_searchTerm = req.section(' ', 1, -1, QString::SectionSkipEmpty);
			res->push_back(w);
		}
}

/**************************************************************************/
void WebSearch::queryFallback(const QString &req, QVector<Service::Item *> *res) const
{
	for (Item *w : _searchEngines){
		w->_searchTerm = req;
		res->push_back(w);
	}
}

/**************************************************************************/
void WebSearch::defaultSearch(const QString &term) const
{
	QDesktopServices::openUrl(QUrl(QString(_searchEngines[0]->_url).replace("%s", term)));
}

/**************************************************************************/
QString WebSearch::defaultSearchText(const QString &term) const{
	return QString("Search for '%1' in %2.").arg(term).arg(_searchEngines[0]->_name);
}
