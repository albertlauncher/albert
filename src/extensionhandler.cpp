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
#include "pluginhandler.h"

/** ***************************************************************************/
ExtensionHandler::ExtensionHandler() {
}



/** ***************************************************************************/
ExtensionHandler::~ExtensionHandler() {
}



/** ***************************************************************************/
void ExtensionHandler::startQuery(const QString &term) {
	_lastSearchTerm = term.trimmed();
	Query *q;
    if (_recentQueries.contains(_lastSearchTerm)){
        q = _recentQueries.value(_lastSearchTerm);
    } else {
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
    this->setSourceModel(q);
}



/** ***************************************************************************/
void ExtensionHandler::setupSession() {
	for (ExtensionInterface *e : _extensions)
 		e->setupSession();
}



/** ***************************************************************************/
void ExtensionHandler::teardownSession() {
	for (ExtensionInterface *e : _extensions)
		e->teardownSession();
    this->setSourceModel(nullptr);
	for (Query *q : _recentQueries)
        delete q;
    _recentQueries.clear();
}



/** ***************************************************************************/
void ExtensionHandler::registerExtension(QObject *o) {
    ExtensionInterface* e = qobject_cast<ExtensionInterface*>(o);
    if (e){
        if(_extensions.contains(e))
            qCritical() << "Extension registered twice!";
        else{
            _extensions.insert(e);
            e->initialize();
        }
    }
}



/** ***************************************************************************/
void ExtensionHandler::unregisterExtension(QObject *o) {
    ExtensionInterface* e = qobject_cast<ExtensionInterface*>(o);
    if (e){
        if(!_extensions.contains(e))
            qCritical() << "Unregistered unregistered extension! (Duplicate unregistration?)";
        else{
            _extensions.remove(e);
            e->finalize();
        }
    }
}
