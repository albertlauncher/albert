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

#include <QFutureSynchronizer>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrent>
#include <chrono>
#include "extension.h"
#include "extensionmanager.h"
#include "query.h"
#include "querymanager.h"

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

//    // Call all setup routines
//    for (QueryHandler *handler : pluginManager_->pluginsByType<ISyncQueryHandler>())
//        handler->setupSession();


//    // Call all setup routines
//    std::chrono::system_clock::time_point start, end;
//    for (Extension *e : extensionManager_->extensions()){
//        start = std::chrono::system_clock::now();
//        e->setupSession();
//        end = std::chrono::system_clock::now();
//        if (50 < std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count())
//            qWarning() << e->id << "took over 50 ms to setup!";
//    }
}

/** ***************************************************************************/
void QueryManager::teardownSession() {

//    // Call all teardown routines
//    std::chrono::system_clock::time_point start, end;
//    for (Extension *e : extensionManager_->extensions()){
//        start = std::chrono::system_clock::now();
//        e->teardownSession();
//        end = std::chrono::system_clock::now();
//        if (50 < std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count())
//            qWarning() << e->id << "took over 50 ms to teardown!";
//    }

    // Delete all the queries of this session
    for (Query* qp : pastQueries_)
        if ( qp->isRunning() )
            connect(qp, &Query::finished, qp, &Query::deleteLater);
        else
            delete qp/*->deleteLater()*/;
    pastQueries_.clear();

    // Compute new match rankings
    Core::MatchOrder::update();
}

/** ***************************************************************************/
void QueryManager::startQuery(const QString &searchTerm) {

    if ( currentQuery_ != nullptr ) {
        // Stop last query
        disconnect(currentQuery_, &Query::resultsReady, this, &QueryManager::resultsReady);
        currentQuery_->invalidate();
        // Store old queries an delete on session teardown (listview needs the model)
        pastQueries_.push_back(currentQuery_);
    }

    // Do nothing if nothing is loaded
    if (extensionManager_->extensions().empty())
        return;

    // Start new query, if not empty
    if ( searchTerm.trimmed().isEmpty() ) {
        currentQuery_ = nullptr;
        emit resultsReady(nullptr);
    } else {
        currentQuery_ = new Query(searchTerm, extensionManager_->extensions());
        connect(currentQuery_, &Query::resultsReady, this, &QueryManager::resultsReady);
    }
}
