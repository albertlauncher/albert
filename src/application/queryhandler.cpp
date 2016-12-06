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

#include <QSqlDriver>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>
#include <chrono>

#include "queryhandler.h"

#include "abstractextension.h"
#include "extensionmanager.h"
#include "query_p.h"


/** ***************************************************************************/
QueryHandler::QueryHandler(ExtensionManager* em, QObject *parent)
    : QObject(parent),
      extensionManager_(em),
      currentQuery_(nullptr) {
    // Initialize the order
    MatchOrder::update();
}

/** ***************************************************************************/
void QueryHandler::setupSession() {
    // Call all setup routines
    std::chrono::system_clock::time_point start, end;
    for (AbstractExtension *e : extensionManager_->extensions()){
        start = std::chrono::system_clock::now();
        e->setupSession();
        end = std::chrono::system_clock::now();
        if (50 < std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count())
            qWarning() << e->id << "took over 50 ms to setup!";
    }
}

/** ***************************************************************************/
void QueryHandler::teardownSession() {

    // Call all teardown routines
    std::chrono::system_clock::time_point start, end;
    for (AbstractExtension *e : extensionManager_->extensions()){
        start = std::chrono::system_clock::now();
        e->teardownSession();
        end = std::chrono::system_clock::now();
        if (50 < std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count())
            qWarning() << e->id << "took over 50 ms to teardown!";
    }

    // Delete all the queries of this session
    for (QueryPrivate* qp : pastQueries_)
        if ( qp->isRunning() )
            connect(qp, &QueryPrivate::finished, qp, &QueryPrivate::deleteLater);
        else
            delete qp/*->deleteLater()*/;
    pastQueries_.clear();

    // Compute new match rankings
    MatchOrder::update();
}

/** ***************************************************************************/
void QueryHandler::startQuery(const QString &searchTerm) {

    if ( currentQuery_ != nullptr ) {
        // Stop last query
        disconnect(currentQuery_, &QueryPrivate::resultsReady, this, &QueryHandler::resultsReady);
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
        currentQuery_ = new QueryPrivate(searchTerm, extensionManager_->extensions());
        connect(currentQuery_, &QueryPrivate::resultsReady, this, &QueryHandler::resultsReady);
    }
}
