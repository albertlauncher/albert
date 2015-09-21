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

#include <QDebug>
#include "extensionmanager.h"
#include "query.h"
#include "interfaces/iextension.h"


/** ***************************************************************************/
ExtensionManager::ExtensionManager() : _sessionIsActive(false) {

}



/** ***************************************************************************/
void ExtensionManager::startQuery(const QString &term) {
    Query *q;
	_currentSearchTerm = term.trimmed();
    if (_recentQueries.contains(_currentSearchTerm)) {
        q = _recentQueries.value(_currentSearchTerm);
    } else {
        q = new Query(_currentSearchTerm);
        _recentQueries.insert(_currentSearchTerm, q);

        // TODO INTRODUCE MULTITHREADING HERE
        for (IExtension *e : _extensions)
            e->handleQuery(q);

        // Wait for highspeed threads to finish

        // Sort
        q->sort();

        // Enable dynmic mode so that lame plugings can still add sorted
    }
    emit newModel(q);
}



/** ***************************************************************************/
void ExtensionManager::setupSession() {
    _sessionIsActive = true;
    for (IExtension *e : _extensions)
 		e->setupSession();
}



/** ***************************************************************************/
void ExtensionManager::teardownSession() {
    for (IExtension *e : _extensions)
        e->teardownSession();
    emit newModel(nullptr);
    for (Query *q : _recentQueries)
        delete q;
    _recentQueries.clear();
    _sessionIsActive = false;
}



/** ***************************************************************************/
void ExtensionManager::registerExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e) {
        if(_extensions.contains(e))
            qCritical() << "Extension registered twice!";
        else{
            _extensions.insert(e);
            e->initialize(this);
        }
    }
}



/** ***************************************************************************/
void ExtensionManager::unregisterExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e) {
        if(!_extensions.contains(e))
            qCritical() << "Unregistered unregistered extension! (Duplicate unregistration?)";
        else{
            _extensions.remove(e);
            e->finalize();
        }
    }
}



/** ***************************************************************************/
void ExtensionManager::activate(const QModelIndex &index) {
    _recentQueries[_currentSearchTerm]->activate(index);
}



/** ***************************************************************************/
bool ExtensionManager::sessionIsActive() const {
    return _sessionIsActive;
}
