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
        currentQuery_ = nullptr;
        emit newModel(nullptr);
        return;
    }

//    // Skip if essentially nothing changed
//    if (currentQuery_ && trimmedTerm==currentQuery_->searchTerm())
//        return;

    //  ▼ TODO INTRODUCE MULTITHREADING HERE ▼
    currentQuery_ = std::make_shared<Query>(trimmedTerm);

//    if (!fallbackOnly) {

        // Separate the first section of the searchterm
        QString potentialTrigger = trimmedTerm.section(' ', 0,0);

        // Check if the trigger matches any exclusive extension and run it
        for (IExtension *e : extensions_)
            if (e->runExclusive())
                for (const QString& trigger : e->triggers())
                    if (trigger == potentialTrigger){
                        e->handleQuery(currentQuery_);
                        emit newModel(currentQuery_->impl);
                        return;
                    }

        // Iterate over the triggers of trigger-extensions
        for (IExtension *e : extensions_){
            if (!e->runExclusive()){
                if (e->triggers().isEmpty()){  // Non triggered extension
                    e->handleQuery(currentQuery_);
                } else {  // Extension that requires a trigger
                    for (const QString& trigger : e->triggers())
                        if (trigger == potentialTrigger)
                            e->handleQuery(currentQuery_);
                }
            }
        }

        // This is a conceptual hack for v0.7, the query should sor itself when the
        // remove friend query  and query_p
        std::stable_sort(currentQuery_->impl->matches_.begin(),
                         currentQuery_->impl->matches_.end(),
                         [](const Match &lhs, const Match &rhs) {
                            return lhs.item.data->urgency() > rhs.item.data->urgency() // Urgency, for e.g. notifications, Warnings
                                    || lhs.item.data->usageCount() > rhs.item.data->usageCount() // usage count
                                    || lhs.score > rhs.score; // percentual match of the query against the item
                         });
//    }

    // TODO Handle this with proper fallbacks
    // Fallback query if results are empty
    if (currentQuery_->impl->matches_.size()==0)
        for (IExtension *e : extensions_)
            e->handleFallbackQuery(currentQuery_);

    emit newModel(currentQuery_->impl);
}



/** ***************************************************************************/
void ExtensionManager::setupSession() {
    for (IExtension *e : extensions_)
 		e->setupSession();
}



/** ***************************************************************************/
void ExtensionManager::teardownSession() {
    for (IExtension *e : extensions_)
        e->teardownSession();
    emit newModel(nullptr);
}



/** ***************************************************************************/
void ExtensionManager::registerExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e) {
        if(extensions_.count(e))
            qCritical() << "Extension registered twice!";
        else
            extensions_.insert(e);
    }
}



/** ***************************************************************************/
void ExtensionManager::unregisterExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e) {
        if(!extensions_.count(e))
            qCritical() << "Unregistered unregistered extension! (Duplicate unregistration?)";
        else
            extensions_.erase(e);
    }
}



/** ***************************************************************************/
void ExtensionManager::activate(const QModelIndex &index) {
    currentQuery_->impl->activate(index);
}
