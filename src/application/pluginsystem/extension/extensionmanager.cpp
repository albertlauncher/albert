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
ExtensionManager::ExtensionManager() {}



/** ***************************************************************************/
void ExtensionManager::startQuery(const QString &searchTerm) {
    // Trim spaces
    QString trimmedTerm = searchTerm.trimmed();

    // Ignore empty queries
    if (trimmedTerm.isEmpty()){
        emit newModel(nullptr);
        return;
    }

    currentQuery_ = std::make_shared<Query>(trimmedTerm);

    //  ▼ TODO INTRODUCE MULTITHREADING HERE ▼

    // Separate the first section of the searchterm
    QString potentialTrigger = trimmedTerm.section(' ', 0,0);

    // Iterate over the triggers of trigger-extensions
    for (IExtension *e : extensions_) {
        if (e->isTriggerOnly()) {
            for (const QString& tr : e->triggers()) {

                // If the trigger matches the first section, run the query
                if (tr == potentialTrigger) {

                    // ▼ TODO RUN IN BACKGROUND ▼
                    e->handleQuery(currentQuery_);
                    // ▲ RUN THIS IN BACKGROUND ▲

                    //  If this extension wants to be run exclusively, return
                    if (e->runExclusive()){
                        emit newModel(currentQuery_->impl);
                        return;
                    }
                }
            }
        }
    }

    // Query the static offline index
    // TODO: Should return matches with precomputed scores
    for (shared_ptr<AlbertItem> obj : offlineIndex_.search(currentQuery_->searchTerm()))
        currentQuery_->addMatch(obj, 0); // TODO How to respect usage?

    // Query all nontrigger-extensions
    for (IExtension *e : extensions_) {
        if (!e->isTriggerOnly()){
            // ▼ TODO RUN IN BACKGROUND ▼
            e->handleQuery(currentQuery_);
            // ▲ RUN THIS IN BACKGROUND ▲
        }
    }

    // TODO Handle this with proper fallbacks
    // Fallback query if results are empty
    if (currentQuery_->impl->matches_.size()==0)
        for (auto &e : extensions_)
            e->handleFallbackQuery(currentQuery_);
    else
        // This is a conceptual hack for v0.7, the query should sor itself when the
        // remove friend query  and query_p
        std::stable_sort(currentQuery_->impl->matches_.begin(),
                         currentQuery_->impl->matches_.end(),
                         [](const Match &lhs, const Match &rhs) {
                            return lhs.score > rhs.score;
                         });


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
    if (e){
        if(!extensions_.count(e)){
            extensions_.insert(e);
            connect(e, &IExtension::staticItemsChanged,
                    this, &ExtensionManager::buildOfflineIndex);
        }
        else
            qCritical() << "Extension registered twice!";
    }
}



/** ***************************************************************************/
void ExtensionManager::unregisterExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e){
        if(extensions_.count(e)){
            extensions_.erase(e);
            disconnect(e, &IExtension::staticItemsChanged,
                       this, &ExtensionManager::buildOfflineIndex);
        }
        else
            qCritical() << "Unregistered unregistered extension! (Duplicate unregistration?)";
    }
}



/** ***************************************************************************/
void ExtensionManager::activate(const QModelIndex &index) {
    currentQuery_->impl->activate(index);
}



/** ***************************************************************************/
void ExtensionManager::buildOfflineIndex() {
    // NOTE: Could be nicer
    offlineIndex_.clear();
    for (IExtension *e : extensions_) {
        for (shared_ptr<AlbertItem> &ai : e->staticItems()) {
            offlineIndex_.add(ai);
        }
    }
}
