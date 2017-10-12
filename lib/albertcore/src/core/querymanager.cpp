// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <chrono>
#include <vector>
#include "extension.h"
#include "extensionmanager.h"
#include "item.h"
#include "matchcompare.h"
#include "queryexecution.h"
#include "querymanager.h"
#include "queryhandler.h"
#include "fallbackprovider.h"
using namespace Core;
using namespace std;
using namespace std::chrono;

namespace {
const char* CFG_INCREMENTAL_SORT = "incrementalSort";
const bool  DEF_INCREMENTAL_SORT = false;
}

/** ***************************************************************************/
Core::QueryManager::QueryManager(ExtensionManager* em, QObject *parent)
    : QObject(parent),
      extensionManager_(em) {

    // Initialize the order
    Core::MatchCompare::update();

    QSettings s(qApp->applicationName());
    incrementalSort_ = s.value(CFG_INCREMENTAL_SORT, DEF_INCREMENTAL_SORT).toBool();
}


/** ***************************************************************************/
QueryManager::~QueryManager() {

}


/** ***************************************************************************/
void Core::QueryManager::setupSession() {

    qDebug() << "========== SESSION SETUP STARTED ==========";

    system_clock::time_point start = system_clock::now();

    // Call all setup routines
    for (Core::QueryHandler *handler : extensionManager_->queryHandlers()) {
        system_clock::time_point start = system_clock::now();
        handler->setupSession();
        long duration = duration_cast<microseconds>(system_clock::now()-start).count();
        qDebug() << qPrintable(QString("TIME: %1 µs SESSION SETUP [%2]").arg(duration, 6).arg(handler->id));
    }

    long duration = duration_cast<microseconds>(system_clock::now()-start).count();
    qDebug() << qPrintable(QString("TIME: %1 µs SESSION SETUP OVERALL").arg(duration, 6));
}


/** ***************************************************************************/
void Core::QueryManager::teardownSession() {

    qDebug() << "========== SESSION TEARDOWN STARTED ==========";

    system_clock::time_point start = system_clock::now();

    // Call all teardown routines
    for (Core::QueryHandler *handler : extensionManager_->queryHandlers()) {
        system_clock::time_point start = system_clock::now();
        handler->teardownSession();
        long duration = duration_cast<microseconds>(system_clock::now()-start).count();
        qDebug() << qPrintable(QString("TIME: %1 µs SESSION TEARDOWN [%2]").arg(duration, 6).arg(handler->id));
    }

    // Clear views
    emit resultsReady(nullptr);

    // Store statistics
    for ( QueryExecution *query : pastQueries_ )
        Statistics::addRecord(query->stats);
    Statistics::commitRecords();


    // Delete queries
    for ( QueryExecution *query : pastQueries_ )
        if ( query->state() == QueryExecution::State::Running )
            connect(query, &QueryExecution::stateChanged,
                    query, [query](){ query->deleteLater(); });
        else
            delete query;
    pastQueries_.clear();

    // Compute new match rankings
    Core::MatchCompare::update();

    long duration = duration_cast<microseconds>(system_clock::now()-start).count();
    qDebug() << qPrintable(QString("TIME: %1 µs SESSION TEARDOWN OVERALL").arg(duration, 6));
}


/** ***************************************************************************/
void Core::QueryManager::startQuery(const QString &searchTerm) {

    qDebug() << "========== QUERY:" << searchTerm << " ==========";

    if ( pastQueries_.size() ) {
        // Stop last query
        QueryExecution *last = pastQueries_.back();
        disconnect(last, &QueryExecution::resultsReady, this, &QueryManager::resultsReady);
        if (last->state() != QueryExecution::State::Finished)
            last->cancel();
    }

    system_clock::time_point start = system_clock::now();

    // Start query
    QueryExecution *currentQuery = new QueryExecution(extensionManager_->queryHandlers(),
                                                      extensionManager_->fallbackProviders(),
                                                      searchTerm, incrementalSort_);
    connect(currentQuery, &QueryExecution::resultsReady, this, &QueryManager::resultsReady);
    currentQuery->run();

    connect(currentQuery, &QueryExecution::stateChanged, [start](QueryExecution::State state){
        if ( state == QueryExecution::State::Finished ) {
            long duration = duration_cast<microseconds>(system_clock::now()-start).count();
            qDebug() << qPrintable(QString("TIME: %1 µs QUERY OVERALL").arg(duration, 6));
        }
    });

    pastQueries_.emplace_back(currentQuery);


    long duration = duration_cast<microseconds>(system_clock::now()-start).count();
    qDebug() << qPrintable(QString("TIME: %1 µs SESSION TEARDOWN OVERALL").arg(duration, 6));
}


/** ***************************************************************************/
bool QueryManager::incrementalSort(){
    return incrementalSort_;
}


/** ***************************************************************************/
void QueryManager::setIncrementalSort(bool value){
    QSettings(qApp->applicationName()).setValue(CFG_INCREMENTAL_SORT, value);
    incrementalSort_ = value;
}
