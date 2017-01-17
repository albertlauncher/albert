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
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <vector>
#include "extension.h"
#include "extensionmanager.h"
#include "fallbackprovider.h"
#include "item.h"
#include "matchcompare.h"
#include "query.h"
#include "queryhandler.h"
#include "querymanager.h"
using namespace Core;
using std::set;
using std::vector;
using std::shared_ptr;

/** ***************************************************************************/
QueryManager::QueryManager(ExtensionManager* em, QObject *parent)
    : QObject(parent),
      extensionManager_(em),
      currentQuery_(nullptr) {

    // Initialize the order
    Core::MatchCompare::update();
}



/** ***************************************************************************/
void QueryManager::setupSession() {
    // Call all setup routines
    for (Core::QueryHandler *handler : extensionManager_->objectsByType<Core::QueryHandler>())
        handler->setupSession();
}



/** ***************************************************************************/
void QueryManager::teardownSession() {

    // Call all teardown routines
    for (Core::QueryHandler *handler : extensionManager_->objectsByType<Core::QueryHandler>())
        handler->teardownSession();

    // Open database to store the runtimes
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery sqlQuery;
    db.transaction();

    // Delete finished queries and prepare sql transaction containing runtimes
    vector<Query*>::iterator it = pastQueries_.begin();
    while ( it != pastQueries_.end()){
        if ( (*it)->state() != Query::State::Running ) {

            // Store the runtimes
            for ( auto &queryHandlerRuntimeEntry : (*it)->runtimes() ) {
                sqlQuery.prepare("INSERT INTO runtimes (extensionId, runtime) VALUES (:extensionId, :runtime);");
                sqlQuery.bindValue(":extensionId", queryHandlerRuntimeEntry.first->id);
                sqlQuery.bindValue(":runtime", static_cast<qulonglong>(queryHandlerRuntimeEntry.second));
                if (!sqlQuery.exec())
                    qWarning() << sqlQuery.lastError();
            }

            // Delete the query
            (*it)->deleteLater();
            it = pastQueries_.erase(it);
        } else
            ++it;
    }

    // Finally send the sql transaction
    db.commit();

    // Compute new match rankings
    Core::MatchCompare::update();
}



/** ***************************************************************************/
void QueryManager::startQuery(const QString &searchTerm) {

    if ( currentQuery_ != nullptr ) {
        // Stop last query
        disconnect(currentQuery_, &Query::resultsReady, this, &QueryManager::resultsReady);
        currentQuery_->invalidate();
        // Store for later deletion (listview still has the model)
        pastQueries_.push_back(currentQuery_);
    }

    // Do nothing if nothing is loaded
    if ( extensionManager_->objects().empty() )
        return;

    // Do nothing if query is empty
    if ( searchTerm.trimmed().isEmpty() ) {
        currentQuery_ = nullptr;
        emit resultsReady(nullptr);
        return;
    }

    // Get fallbacks
    vector<shared_ptr<Item>> fallbacks;
    for ( FallbackProvider *extension : extensionManager_->objectsByType<FallbackProvider>() ) {
        vector<shared_ptr<Item>> && tmpFallbacks = extension->fallbacks(searchTerm);
        fallbacks.insert(fallbacks.end(),
                         std::make_move_iterator(tmpFallbacks.begin()),
                         std::make_move_iterator(tmpFallbacks.end()));
    }

    // Determine query handlers
    const set<QueryHandler*> allHandlers = extensionManager_->objectsByType<QueryHandler>();
    set<QueryHandler*> actualHandlers;
    for ( QueryHandler *handler : allHandlers )
        if ( !handler->trigger().isEmpty() && searchTerm.startsWith(handler->trigger()) )
            actualHandlers.insert(handler);

    if (actualHandlers.empty())
        for ( QueryHandler *handler : allHandlers )
            if ( handler->trigger().isEmpty() )
                actualHandlers.insert(handler);


    // Start query
    currentQuery_ = new Query;
    connect(currentQuery_, &Query::resultsReady, this, &QueryManager::resultsReady);
    currentQuery_->setSearchTerm(searchTerm);
    currentQuery_->setQueryHandlers(actualHandlers);
    currentQuery_->setFallbacks(fallbacks);
    currentQuery_->run();
}
