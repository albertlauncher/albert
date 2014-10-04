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

#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QDebug>

/**************************************************************************/
void WebSearch::initialize()
{

	QStringList engines = QSettings().value(QString::fromLocal8Bit("search_engines")).toStringList();
	for (QString &e : engines)
	{
		QStringList engineComponents = e.split('|');
		if( engineComponents.size() == 4){
			Item *i = new Item;
			i->_lastAccess = 1;
			i->_name       = engineComponents[0];
			i->_url        = engineComponents[1];
			i->_shortcut   = engineComponents[2];
			i->_iconPath   = engineComponents[3];
			_searchEngines.push_back(i);
		}
	}
}

/**************************************************************************/
QDataStream &WebSearch::serialize(QDataStream &out) const
{
	out << _searchEngines.size();
	for (WebSearch::Item* it : _searchEngines)
		 it->serialize(out);
	return out;
}

/**************************************************************************/
QDataStream &WebSearch::deserialize(QDataStream &in)
{
	int size;
	in >> size;
	for (int i = 0; i < size; ++i) {
		WebSearch::Item *it = new WebSearch::Item;
		it->deserialize(in);
		_searchEngines.push_back(it);
	}
	return in;
}

/**************************************************************************/
void WebSearch::query(const QString &req, QVector<Service::Item *> *res) const noexcept
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
void WebSearch::queryAll(const QString &req, QVector<Service::Item *> *res)
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
