// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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

#include "extensionhandler.h"
#include <QDirIterator>
#include <QDebug>
#include <QJsonArray>
#include "settings.h"
#include "pluginhandler.h"

/****************************************************************************///
void ExtensionHandler::startQuery(const QString &term)
{
	_lastSearchTerm = term.trimmed();

	Query *q;
    if (_recentQueries.contains(_lastSearchTerm)){
        qDebug() << "Query loaded" << _lastSearchTerm;
        q = _recentQueries.value(_lastSearchTerm);
    } else {
        qDebug() << "Query started" << _lastSearchTerm;
        q = new Query(_lastSearchTerm);
        _recentQueries.insert(_lastSearchTerm, q);

        // TODO INTRODUCE MULTITHREADING HERE
        for (ExtensionInterface *e : _extensions)
            e->handleQuery(q);

        // Wait for highspeed threads to finish

        // Sort
        q->sort();

        // Enable dynmic mode so that lame plugings can still add sorted
    }
	emit currentQueryChanged(q);
}

/****************************************************************************///
void ExtensionHandler::initialize()
{
    QList<ExtensionInterface*> extensions =
            PluginHandler::instance()->getLoadedPlugins<ExtensionInterface*>();

    qDebug() << "Initialize extenstions.";
    for (ExtensionInterface *e : extensions){
        if (_extensions.contains(e))
            continue;
        _extensions.insert(e);
        e->initialize();
    }
}

/****************************************************************************///
void ExtensionHandler::finalize()
{
	qDebug() << "Finalize extenstions.";
	for (ExtensionInterface *e : _extensions)
		e->finalize();
}

/****************************************************************************///
void ExtensionHandler::setupSession(){
	for (ExtensionInterface *e : _extensions)
		e->setupSession();
	qDebug() << "Session set up.";
}

/****************************************************************************///
void ExtensionHandler::teardownSession(){
	_recentQueries.clear();
	for (Query *q : _recentQueries)
		delete q;
	for (ExtensionInterface *e : _extensions)
		e->teardownSession();
	qDebug() << "Session teared down.";
}
