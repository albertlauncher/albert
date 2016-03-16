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
#include "query_p.hpp"
#include "iextension.h"


/** ***************************************************************************/
void ExtensionManager::startQuery(const QString &searchTerm) {
    // Trim spaces
    QString trimmedTerm = searchTerm.trimmed().toLower();

    // Ignore empty queries
    if (trimmedTerm.isEmpty()){
        emit newModel(nullptr);
        return;
    }

    _currentQuery = std::make_shared<Query>(trimmedTerm);

    //  ▼ TODO INTRODUCE MULTITHREADING HERE ▼

    // Separate the first section of the searchterm
    QString potentialTrigger = trimmedTerm.section(' ', 0,0);

    // Iterate over the triggers of trigger-extensions
    for (IExtension *e : _extensions) {
        if (e->isTriggerOnly()) {
            for (const QString& trigger : e->triggers()) {
                // If the trigger matches the first section, run the query
                if (trigger == potentialTrigger) {
                    e->handleQuery(_currentQuery);
                    //  If this extension wants to be run exclusively, return
                    if (e->runExclusive()){
                        emit newModel(_currentQuery->impl);
                        return;
                    }
                }
            }
        }
    }

    // Query all nontrigger-extensions
    for (IExtension *e : _extensions)
        if (!e->isTriggerOnly())
            e->handleQuery(_currentQuery);

    // TODO Handle this with proper fallbacks
    // Fallback query if results are empty
    if (_currentQuery->impl->matches_.size()==0)
        for (IExtension *e : _extensions)
            e->handleFallbackQuery(_currentQuery);
    else
        // This is a conceptual hack for v0.7, the query should sor itself when the
        // remove friend query  and query_p
        std::stable_sort(_currentQuery->impl->matches_.begin(),
                         _currentQuery->impl->matches_.end(),
                         [](const Match &lhs, const Match &rhs) {
                            return lhs.item.data->importance() > rhs.item.data->importance() // Importance, for e.g. urgent notifications
                                    || lhs.item.data->usageCount() > rhs.item.data->usageCount() // usage count
                                    || lhs.score > rhs.score; // percentual match of the query against the item
                         });

    //  ▲ INTRODUCE MULTITHREADING HERE ▲

    emit newModel(_currentQuery->impl);
}



/** ***************************************************************************/
void ExtensionManager::setupSession() {
    for (IExtension *e : _extensions)
 		e->setupSession();
}



/** ***************************************************************************/
void ExtensionManager::teardownSession() {
    for (IExtension *e : _extensions)
        e->teardownSession();
    emit newModel(nullptr);
}



/** ***************************************************************************/
void ExtensionManager::registerExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e) {
        if(_extensions.count(e))
            qCritical() << "Extension registered twice!";
        else
            _extensions.insert(e);
    }
}



/** ***************************************************************************/
void ExtensionManager::unregisterExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e) {
        if(!_extensions.count(e))
            qCritical() << "Unregistered unregistered extension! (Duplicate unregistration?)";
        else
            _extensions.erase(e);
    }
}



/** ***************************************************************************/
void ExtensionManager::activate(const QModelIndex &index) {
    _currentQuery->impl->activate(index);
}
