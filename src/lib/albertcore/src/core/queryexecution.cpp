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

#include <QDebug>
#include <chrono>
#include <algorithm>
#include <utility>
#include "queryhandler.h"
#include "fallbackprovider.h"
#include "queryexecution.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QString>
#include <QtConcurrent>
#include <QVariant>
#include <algorithm>
#include <functional>
#include "action.h"
#include "matchcompare.h"
#include "item.h"
#include "itemroles.h"
using std::chrono::system_clock;
using namespace std;


/** ***************************************************************************/
Core::QueryExecution::QueryExecution(const std::set<QueryHandler*> & queryHandlers,
                                     const std::set<FallbackProvider*> &fallbackProviders,
                                     const QString &searchTerm) {

    query_.searchTerm_ = searchTerm;

    // Run with a single handler if the trigger matches
    for ( QueryHandler *handler : queryHandlers ) {
        for ( const QString& trigger : handler->triggers() ) {
            if ( !trigger.isEmpty() && searchTerm.startsWith(trigger) ) {
                query_.trigger_ = trigger;
                ( handler->executionType()==QueryHandler::ExecutionType::Batch )
                        ? batchHandlers_.insert(handler)
                        : realtimeHandlers_.insert(handler);
                return;
            }
        }
    }

    // Else run all handlers
    for ( QueryHandler *queryHandler : queryHandlers )
        ( queryHandler->executionType()==QueryHandler::ExecutionType::Batch )
                ? batchHandlers_.insert(queryHandler)
                : realtimeHandlers_.insert(queryHandler);

    // Get fallbacks
    for ( FallbackProvider *fallbackProvider : fallbackProviders ) {
        vector<shared_ptr<Item>> && tmpFallbacks = fallbackProvider->fallbacks(searchTerm);
        fallbacks_.insert(fallbacks_.end(),
                          std::make_move_iterator(tmpFallbacks.begin()),
                          std::make_move_iterator(tmpFallbacks.end()));
    }
}


/** ***************************************************************************/
Core::QueryExecution::~QueryExecution() {

}


/** ***************************************************************************/
const Core::Query *Core::QueryExecution::query() {
    return &query_;
}


/** ***************************************************************************/
const Core::QueryExecution::State &Core::QueryExecution::state() const {
    return state_;
}


/** ***************************************************************************/
void Core::QueryExecution::setState(State state) {
    state_ = state;
    emit stateChanged(state_);
}


/** ***************************************************************************/
int Core::QueryExecution::rowCount(const QModelIndex &) const {
    return static_cast<int>(results_.size());
}


/** ***************************************************************************/
QHash<int,QByteArray> Core::QueryExecution::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[static_cast<int>(ItemRoles::TextRole)]       = "itemTextRole";
    roles[static_cast<int>(ItemRoles::ToolTipRole)]    = "itemToolTipRole";
    roles[static_cast<int>(ItemRoles::DecorationRole)] = "itemDecorationRole";
    roles[static_cast<int>(ItemRoles::CompletionRole)] = "itemCompletionStringRole";
    roles[static_cast<int>(ItemRoles::ActionRole)]     = "itemActionRole"; // used to activate items as well
    roles[static_cast<int>(ItemRoles::AltActionRole)]  = "itemAltActionsRole";  // used to activate items as well
    roles[static_cast<int>(ItemRoles::FallbackRole)]   = "itemFallbackRole"; // used to activate items as well
    return roles;
}


/** ***************************************************************************/
QVariant Core::QueryExecution::data(const QModelIndex &index, int role) const {
    if (index.isValid()) {
        const shared_ptr<Item> &item = results_[static_cast<size_t>(index.row())];

        switch ( role ) {
        case ItemRoles::TextRole:
            return item->text();
        case ItemRoles::ToolTipRole:
            return item->subtext();
        case ItemRoles::DecorationRole:
            return item->iconPath();
        case ItemRoles::CompletionRole:
            return item->completionString();
        case ItemRoles::ActionRole:
            return (0 < static_cast<int>(item->actions().size()))
                    ? item->actions()[0]->text()
                    : item->subtext();
        case ItemRoles::AltActionRole: { // Actions list
            QStringList actionTexts;
            for (const shared_ptr<Action> &action : item->actions())
                actionTexts.append(action->text());
            return actionTexts;
        }
        case ItemRoles::FallbackRole:
            return "Search '"+query_.searchTerm()+"' using default fallback";
        }
    }
    return QVariant();
}



/** ***************************************************************************/
bool Core::QueryExecution::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (index.isValid()) {
        shared_ptr<Item> &item = results_[static_cast<size_t>(index.row())];
        QString activateditemId;

        switch ( role ) {
        case ItemRoles::ActionRole:{
            if (0U < item->actions().size()){
                item->actions()[0]->activate();
                activateditemId = item->id();
            }
            break;
        }
        case ItemRoles::AltActionRole:{
            size_t actionValue = static_cast<size_t>(value.toInt());
            if (actionValue < item->actions().size()) {
                item->actions()[actionValue]->activate();
                activateditemId = item->id();
            }
            break;
        }
        case ItemRoles::FallbackRole:{
            if (0U < fallbacks_.size() && 0U < item->actions().size()) {
                fallbacks_[0]->actions()[0]->activate();
                activateditemId = fallbacks_[0]->id();
            }
            break;
        }
        }

        if ( !activateditemId.isNull() ) {
            // Save usage
            QSqlQuery query;
            query.prepare("INSERT INTO usages (input, itemId) VALUES (:input, :itemId);");
            query.bindValue(":input", query_.searchTerm());
            query.bindValue(":itemId", item->id());
            if (!query.exec())
                qWarning() << query.lastError();
            return true;
        }
    }
    return false;
}


/** ***************************************************************************/
const std::map<QString, uint> &Core::QueryExecution::runtimes() {
    return runtimes_;
}


/** ***************************************************************************/
void Core::QueryExecution::run() {

    setState(State::Running);

    if ( !batchHandlers_.empty() )
        return runBatchHandlers();

    emit resultsReady(this);

    if ( !realtimeHandlers_.empty() )
        return runRealitimeHandlers();

    setState(State::Finished);
}


/** ***************************************************************************/
void Core::QueryExecution::cancel() {
    futureWatcher_.disconnect();
    future_.cancel();
    query_.isValid_ = false;
}


/** ***************************************************************************/
void Core::QueryExecution::runBatchHandlers() {

    // Call onBatchHandlersFinished when all handlers finished
    connect(&futureWatcher_, &QFutureWatcher<pair<QueryHandler*,uint>>::finished,
            this, &QueryExecution::onBatchHandlersFinished);

    // Run the handlers concurrently and measure the runtimes
    std::function<pair<QueryHandler*,uint>(QueryHandler*)> func = [this](QueryHandler* queryHandler){
        system_clock::time_point then = system_clock::now();
        queryHandler->handleQuery(&query_);
        system_clock::time_point now = system_clock::now();
        return std::make_pair(queryHandler, std::chrono::duration_cast<std::chrono::microseconds>(now-then).count());
    };
    future_ = QtConcurrent::mapped(batchHandlers_.begin(), batchHandlers_.end(), func);
    futureWatcher_.setFuture(future_);
}


/** ***************************************************************************/
void Core::QueryExecution::onBatchHandlersFinished() {

    // Save the runtimes of the current future
    for ( auto it = future_.begin(); it != future_.end(); ++it )
        runtimes_.emplace(it->first->id, it->second);

    // Sort the results
    query_.mutex_.lock();
    std::stable_sort(query_.results_.begin(),
                     query_.results_.end(),
                     MatchCompare());

    // Move the items of the "pending results" into "results"
    std::transform(query_.results_.begin(),
                   query_.results_.end(),
                   std::back_inserter(results_),
                   [](const pair<shared_ptr<Item>,uint>& p){ return std::move(p.first); });

    query_.results_.clear();
    query_.mutex_.unlock();

    emit resultsReady(this);

    if ( realtimeHandlers_.empty() )
        finishQuery();
    else
        runRealitimeHandlers();
}


/** ***************************************************************************/
void Core::QueryExecution::runRealitimeHandlers() {

    // Call onRealitimeHandlersFinsished when all handlers finished
    disconnect(&futureWatcher_, &QFutureWatcher<pair<QueryHandler*,uint>>::finished,
               this, &QueryExecution::onBatchHandlersFinished);

    connect(&futureWatcher_, &QFutureWatcher<pair<QueryHandler*,uint>>::finished,
            this, &QueryExecution::onRealitimeHandlersFinsished);

    // Run the handlers concurrently and measure the runtimes
    std::function<pair<QueryHandler*,uint>(QueryHandler*)> func = [this](QueryHandler* queryHandler){
        system_clock::time_point then = system_clock::now();
        queryHandler->handleQuery(&query_);
        system_clock::time_point now = system_clock::now();
        return std::make_pair(queryHandler, std::chrono::duration_cast<std::chrono::microseconds>(now-then).count());
    };
    future_ = QtConcurrent::mapped(realtimeHandlers_.begin(), realtimeHandlers_.end(), func);
    futureWatcher_.setFuture(future_);

    // Insert pending results every 50 milliseconds
    connect(&fiftyMsTimer_, &QTimer::timeout, this, &QueryExecution::insertPendingResults);
    fiftyMsTimer_.start(50);
}


/** ***************************************************************************/
void Core::QueryExecution::onRealitimeHandlersFinsished() {

    // Save the runtimes of the current future
    for ( auto it = future_.begin(); it != future_.end(); ++it )
        runtimes_.emplace(it->first->id, it->second);

    // Finally done
    fiftyMsTimer_.stop();
    fiftyMsTimer_.disconnect();
    insertPendingResults();

    finishQuery();
}


/** ***************************************************************************/
void Core::QueryExecution::insertPendingResults() {

    if(query_.results_.size()) {

        QMutexLocker lock(&query_.mutex_);

        beginInsertRows(QModelIndex(),
                        static_cast<int>(results_.size()),
                        static_cast<int>(results_.size()+query_.results_.size()-1));

        // Preallocate space to avoid multiple allocatoins
        results_.reserve(results_.size() + query_.results_.size());

        // Copy the items of the matches into the results
        std::transform(query_.results_.begin(), query_.results_.end(),
                       std::back_inserter(results_),
                       [](const pair<shared_ptr<Item>,uint>& p){ return std::move(p.first); });

        endInsertRows();

        // Clear the empty matches
        query_.results_.clear();
    }
}


/** ***************************************************************************/
void Core::QueryExecution::finishQuery() {

    /*
     * If results are empty show fallbacks
     */

    if( results_.empty() ){
        beginInsertRows(QModelIndex(), 0, static_cast<int>(fallbacks_.size()-1));
        results_.insert(results_.end(),
                       fallbacks_.begin(),
                       fallbacks_.end());
        endInsertRows();
    }
    setState(State::Finished);
}

