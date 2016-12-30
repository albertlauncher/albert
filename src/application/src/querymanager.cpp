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

#include <vector>
#include "querymanager.h"
#include "extensionmanager.h"
#include "queryhandler.h"
#include "query.h"
#include "item.h"
#include "extension.h"
#include "queryhandler.h"
#include "fallbackprovider.h"
using namespace Core;
using std::vector;
using std::shared_ptr;

/** ***************************************************************************/
QueryManager::QueryManager(ExtensionManager* em, QObject *parent)
    : QObject(parent),
      extensionManager_(em),
      currentQuery_(nullptr) {
    // Initialize the order
    Core::MatchOrder::update();
}



/** ***************************************************************************/
void QueryManager::setupSession() {
    // Call all setup routines
    for (Core::QueryHandler *handler : extensionManager_->extensionsByType<Core::QueryHandler>())
        handler->setupSession();
}



/** ***************************************************************************/
void QueryManager::teardownSession() {

    // Call all teardown routines
    for (Core::QueryHandler *handler : extensionManager_->extensionsByType<Core::QueryHandler>())
        handler->teardownSession();

    // Delete finished queries
    vector<Query*>::iterator it = pastQueries_.begin();
    while ( it != pastQueries_.end()){
        if ( (*it)->state() != Query::State::Running ) {
            (*it)->deleteLater();
            it = pastQueries_.erase(it);
        } else
            ++it;
    }

    // Compute new match rankings
    Core::MatchOrder::update();
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
    if ( extensionManager_->extensions().empty() )
        return;

    // Do nothing if query is empty
    if ( searchTerm.trimmed().isEmpty() ) {
        currentQuery_ = nullptr;
        emit resultsReady(nullptr);
        return;
    }

    // Get fallbacks
    vector<shared_ptr<Item>> fallbacks;
    for ( FallbackProvider *extension : extensionManager_->extensionsByType<FallbackProvider>() ) {
        vector<shared_ptr<Item>> && tmpFallbacks = extension->fallbacks(searchTerm);
        fallbacks.insert(fallbacks.end(),
                         std::make_move_iterator(tmpFallbacks.begin()),
                         std::make_move_iterator(tmpFallbacks.end()));
    }

    // Determine query handlers
    const set<QueryHandler*> allHandlers = extensionManager_->extensionsByType<QueryHandler>();
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
