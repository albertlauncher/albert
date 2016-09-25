// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>
#include "extensionmanager.h"
#include "query.h"
#include "query_p.hpp"
#include "iextension.h"


/** ***************************************************************************/
ExtensionManager::ExtensionManager() {
    currentQuery_ = nullptr;
    UXTimeOut_.setInterval(100);
    UXTimeOut_.setSingleShot(true);
    connect(&UXTimeOut_, &QTimer::timeout, this, &ExtensionManager::onUXTimeOut);
}



/** ***************************************************************************/
ExtensionManager::~ExtensionManager() {

}



/** ***************************************************************************/
void ExtensionManager::startQuery(const QString &searchTerm) {

    // Stop last query
    if ( currentQuery_ != nullptr ) {
        disconnect(currentQuery_, &QueryPrivate::finished, this, &ExtensionManager::onQueryFinished);
        currentQuery_->stop();
        pastQueries_.insert(currentQuery_);
    }

    QString trimmedTerm = searchTerm.trimmed();

    // Ignore empty queries
    if (trimmedTerm.isEmpty()){
        currentQuery_ = nullptr;
        emit newModel(nullptr);
        return;
    }

    QString trigger = trimmedTerm.section(' ', 0,0);

    if (triggerExtensions_.count(trigger) != 0){
        // Triggered extesions
        currentQuery_ = new QueryPrivate(trimmedTerm.section(' ', 1), trigger);
        for (IExtension *e : triggerExtensions_[trigger])
            currentQuery_->addHandler(e);
    } else {
        // Untriggered extesions
        currentQuery_ = new QueryPrivate(trimmedTerm);
        for (IExtension *e : extensions_)
            if (!e->runExclusive())
                currentQuery_->addHandler(e);
    }

    // Connect to finished signal and start it
    connect(currentQuery_, &QueryPrivate::finished, this, &ExtensionManager::onQueryFinished);
    currentQuery_->start();
    UXTimeOut_.start();
}



/** ***************************************************************************/
void ExtensionManager::setupSession() {

    // Call all setup routines
    QFutureSynchronizer<void> synchronizer;
    for (IExtension *e : extensions_)
        synchronizer.addFuture(QtConcurrent::run(e, &IExtension::setupSession));
    synchronizer.waitForFinished();

    // Build trigger map
    triggerExtensions_.clear();
    for (IExtension *e : extensions_)
        if (e->runExclusive())
            for (const QString& t : e->triggers())
                triggerExtensions_[t].insert(e);
}



/** ***************************************************************************/
void ExtensionManager::teardownSession() {

    // Call all teardown routines
    QFutureSynchronizer<void> synchronizer;
    for (IExtension *e : extensions_)
        synchronizer.addFuture(QtConcurrent::run(e, &IExtension::teardownSession));
    synchronizer.waitForFinished();

    // Clear the listview
    emit newModel(nullptr);

    // Delete all finished queries
    for (auto it = pastQueries_.begin(); it != pastQueries_.end(); ) {
        if ((*it)->state() == QueryPrivate::State::Finished) {
            (*it)->deleteLater();
            it = pastQueries_.erase(it);
        } else
            ++it;
    }
}



/** ***************************************************************************/
void ExtensionManager::registerExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e) {
        if(extensions_.count(e))
            qCritical() << "Extension registered twice!";
        else {
            extensions_.insert(e);
            updateFallbacks();
            connect(e, &IExtension::fallBacksChanged, this, &ExtensionManager::updateFallbacks);
        }
    }
}



/** ***************************************************************************/
void ExtensionManager::unregisterExtension(QObject *o) {
    IExtension* e = qobject_cast<IExtension*>(o);
    if (e) {
        if(!extensions_.count(e))
            qCritical() << "Unregistered unregistered extension! (Duplicate unregistration?)";
        else {
            extensions_.erase(e);
            disconnect(e, &IExtension::fallBacksChanged, this, &ExtensionManager::updateFallbacks);
        }
    }
}



/** ***************************************************************************/
void ExtensionManager::onUXTimeOut() {

    // Avoid the onFinished procedure if the query timed out before
    disconnect(currentQuery_, &QueryPrivate::finished, this, &ExtensionManager::onQueryFinished);

    // Untriggered queries have to be sorted
    if ( currentQuery_->trigger().isNull() )
        currentQuery_->sort();

    emit newModel(currentQuery_);
}



/** ***************************************************************************/
void ExtensionManager::onQueryFinished(QueryPrivate * qp) {

    // Ignore if this is not the current query
    if ( qp != currentQuery_ )
        return;

    /*
     * If the query finished before the UX timeout timed out everything is fine.
     * If not the results have already been sorted and published. The user sees
     * a list that must not be rearranged for better user experience.
     */

    if (UXTimeOut_.isActive()) {

        // Avoid the timeout procedure
        UXTimeOut_.stop();

        // Untriggered queries have to be sorted
        if ( currentQuery_->trigger().isNull() )
            currentQuery_->sort();

        emit newModel(currentQuery_);
    }

    /*
     * If the query finished and the results are empty, display fallbacks.
     * In this case a sort is okay, since there is no selection to disturb.
     */
    if (currentQuery_->rowCount() == 0) {
        for (auto &it : fallbacks_)
            currentQuery_->addMatch(it);
//        currentQuery_->sort();
    }
}



/** ***************************************************************************/
void ExtensionManager::updateFallbacks() {
    fallbacks_.clear();
    for (auto &ext : extensions_)
        for (auto &fb : ext->fallbacks())
            fallbacks_.push_back(fb);
}
