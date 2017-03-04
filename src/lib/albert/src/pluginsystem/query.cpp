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
#include <QFutureWatcher>
#include <QMutex>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QString>
#include <QtConcurrent>
#include <QTimer>
#include <QVariant>
#include <algorithm>
#include <chrono>
#include <map>
#include <functional>
#include "action.h"
#include "extension.h"
#include "item.h"
#include "matchcompare.h"
#include "query.h"
using std::chrono::system_clock;
using namespace std;


/** ***************************************************************************/
class Core::Query::QueryPrivate : public QAbstractListModel
{
public:
    QueryPrivate(Query *q) : q(q), isValid(true), state(State::Idle) { }

    Query *q;

    QString searchTerm;
    bool isValid;
    Query::State state;

    set<QueryHandler*> syncHandlers;
    set<QueryHandler*> asyncHandlers;
    map<QString,uint> runtimes;

    vector<shared_ptr<Item>> results;
    vector<shared_ptr<Item>> fallbacks;

    QTimer fiftyMsTimer;
    mutable QMutex pendingResultsMutex;
    vector<pair<shared_ptr<Item>, short>> pendingResults;

    QFutureWatcher<pair<QueryHandler*,uint>> futureWatcher;




    /** ***************************************************************************/
    void run() {

        if ( !syncHandlers.empty() )
            return runSyncHandlers();

        emit q->resultsReady(this);

        if ( !asyncHandlers.empty() )
            return runAsyncHandlers();

        state = State::Finished;
        emit q->finished();
    }


    /** ***************************************************************************/
    pair<QueryHandler*,uint> mappedFunction (QueryHandler* queryHandler) {
        system_clock::time_point then = system_clock::now();
        queryHandler->handleQuery(q);
        system_clock::time_point now = system_clock::now();
        return std::make_pair(queryHandler, std::chrono::duration_cast<std::chrono::microseconds>(now-then).count());
    }


    /** ***************************************************************************/
    void runSyncHandlers() {

        // Call onSyncHandlersFinsished when all handlers finished
        futureWatcher.disconnect();
        connect(&futureWatcher, &QFutureWatcher<pair<QueryHandler*,uint>>::finished,
                this, &QueryPrivate::onSyncHandlersFinsished);

        // Run the handlers concurrently and measure the runtimes
        futureWatcher.setFuture(QtConcurrent::mapped(syncHandlers.begin(),
                                                     syncHandlers.end(),
                                                     std::bind(&QueryPrivate::mappedFunction, this, std::placeholders::_1)));
    }


    /** ***************************************************************************/
    void runAsyncHandlers() {

        // Call onAsyncHandlersFinsished when all handlers finished
        futureWatcher.disconnect();
        connect(&futureWatcher, &QFutureWatcher<pair<QueryHandler*,uint>>::finished,
                this, &QueryPrivate::onAsyncHandlersFinsished);

        // Run the handlers concurrently and measure the runtimes
        futureWatcher.setFuture(QtConcurrent::mapped(asyncHandlers.begin(),
                                                     asyncHandlers.end(),
                                                     std::bind(&QueryPrivate::mappedFunction, this, std::placeholders::_1)));

        // Insert pending results every 50 milliseconds
        connect(&fiftyMsTimer, &QTimer::timeout, this, &QueryPrivate::insertPendingResults);
        fiftyMsTimer.start(50);
    }



    /** ***************************************************************************/
    void onSyncHandlersFinsished() {

        // Save the runtimes of the current future
        for ( auto it = futureWatcher.future().begin(); it != futureWatcher.future().end(); ++it )
            runtimes.emplace(it->first->id, it->second);

        /*
         * Publish the results
         */

        // Lock the pending results
        QMutexLocker lock(&pendingResultsMutex);

        // Sort the results
        std::sort(pendingResults.begin(),
                  pendingResults.end(),
                  MatchCompare());

        // Preallocate space in "results" to avoid multiple allocations
        results.reserve(results.size() + pendingResults.size());

        // Move the items of the "pending results" into "results"
        std::transform(pendingResults.begin(),
                       pendingResults.end(),
                       std::back_inserter(results),
                       [](const pair<shared_ptr<Item>,short>& p){ return std::move(p.first); });

        pendingResults.clear();

        emit q->resultsReady(this);

        if ( asyncHandlers.empty() )
            finishQuery();
        else
            runAsyncHandlers();
    }


    /** ***************************************************************************/
    void onAsyncHandlersFinsished() {

        // Save the runtimes of the current future
        for ( auto it = futureWatcher.future().begin(); it != futureWatcher.future().end(); ++it )
            runtimes.emplace(it->first->id, it->second);

        // Finally done
        fiftyMsTimer.stop();
        fiftyMsTimer.disconnect();
        insertPendingResults();

        finishQuery();
    }


    /** ***************************************************************************/
    void insertPendingResults() {

        if(pendingResults.size()) {

            QMutexLocker lock(&pendingResultsMutex);

            beginInsertRows(QModelIndex(), results.size(), results.size() + pendingResults.size() - 1);

            // Preallocate space to avoid multiple allocatoins
            results.reserve(results.size() + pendingResults.size());

            // Copy the items of the matches into the results
            std::transform(pendingResults.begin(), pendingResults.end(),
                           std::back_inserter(results),
                           [](const pair<shared_ptr<Item>,short>& p){ return std::move(p.first); });

            endInsertRows();

            // Clear the empty matches
            pendingResults.clear();
        }
    }


    /** ***************************************************************************/
    void finishQuery() {

        /*
         * If results are empty show fallbacks
         */

        if( results.empty() ){
            beginInsertRows(QModelIndex(), 0, fallbacks.size() - 1);
            results.insert(results.end(),
                           fallbacks.begin(),
                           fallbacks.end());
            endInsertRows();
        }

        state = State::Finished;

        emit q->finished();
    }


    /** ***************************************************************************/
    int rowCount(const QModelIndex &) const override {
        return static_cast<int>(results.size());
    }



    /** ***************************************************************************/
    QVariant data(const QModelIndex &index, int role) const override {
        if (index.isValid()) {
            const shared_ptr<Item> &item = results[static_cast<size_t>(index.row())];

            switch (role) {
            case Qt::DisplayRole:
                return item->text();
            case Qt::ToolTipRole:
                return item->subtext();
            case Qt::DecorationRole:
                return item->iconPath();

            case Qt::UserRole: { // Actions list
                QStringList actionTexts;
                for (const shared_ptr<Action> &action : item->actions())
                    actionTexts.append(action->text());
                return actionTexts;
            }
            case Qt::UserRole+1: // Completion string
                return item->completionString();

            case Qt::UserRole+100: // DefaultAction
                return (0 < static_cast<int>(item->actions().size())) ? item->actions()[0]->text() : item->subtext();
            case Qt::UserRole+101: // AltAction
                return "Search '"+searchTerm+"' using default fallback";
            case Qt::UserRole+102: // MetaAction
                return (1 < static_cast<int>(item->actions().size())) ? item->actions()[1]->text() : item->subtext();
            case Qt::UserRole+103: // ControlAction
                return (2 < static_cast<int>(item->actions().size())) ? item->actions()[2]->text() : item->subtext();
            case Qt::UserRole+104: // ShiftAction
                return (3 < static_cast<int>(item->actions().size())) ? item->actions()[3]->text() : item->subtext();
            default:
                return QVariant();
            }
        }
        return QVariant();
    }



    /** ***************************************************************************/
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (index.isValid()) {
            shared_ptr<Item> &item = results[static_cast<size_t>(index.row())];
            QString itemId = item->id();

            switch (role) {

            // Activation by index
            case Qt::UserRole:{
                size_t actionValue = static_cast<size_t>(value.toInt());
                if (actionValue < item->actions().size())
                    item->actions()[actionValue]->activate();
                break;
            }

            // Activation by modifier
            case Qt::UserRole+100: // DefaultAction
                if (0U < item->actions().size())
                    item->actions()[0]->activate();
                break;
            case Qt::UserRole+101: // Default fallback action (Meta)
                if (0U < fallbacks.size() && 0U < item->actions().size()) {
                    fallbacks[0]->actions()[0]->activate();
                    itemId = fallbacks[0]->id();
                }
                break;
            }

            // Save usage
            QSqlQuery query;
            query.prepare("INSERT INTO usages (input, itemId) VALUES (:input, :itemId);");
            query.bindValue(":input", searchTerm);
            query.bindValue(":itemId", item->id());
            if (!query.exec())
                qWarning() << query.lastError();
        }
        return false;
    }
};





/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Core::Query::Query() : d(new QueryPrivate(this)) {

}


/** ***************************************************************************/
Core::Query::~Query() {

}


/** ***************************************************************************/
const Core::Query::State &Core::Query::state() const {
    return d->state;
}


/** ***************************************************************************/
const QString &Core::Query::searchTerm() const {
    return d->searchTerm;
}


/** ***************************************************************************/
bool Core::Query::isValid() const {
    return d->isValid;
}


/** ***************************************************************************/
void Core::Query::addMatch(shared_ptr<Item> item, short score) {
    if ( d->isValid ) {
        d->pendingResultsMutex.lock();
        d->pendingResults.push_back({item, score});
        d->pendingResultsMutex.unlock();
    }
}


/** ***************************************************************************/
void Core::Query::addMatches(vector<pair<shared_ptr<Item>,short>>::iterator begin,
                             vector<pair<shared_ptr<Item>,short>>::iterator end) {
    if ( d->isValid ) {
        d->pendingResultsMutex.lock();
        d->pendingResults.insert(d->pendingResults.end(),
                                 std::make_move_iterator(begin),
                                 std::make_move_iterator(end));
        d->pendingResultsMutex.unlock();
    }
}


/** ***************************************************************************/
std::map<QString,uint> Core::Query::runtimes() {
    return d->runtimes;
}


/** ***************************************************************************/
void Core::Query::setSearchTerm(const QString &searchTerm) {
    d->searchTerm = searchTerm;
}


/** ***************************************************************************/
void Core::Query::invalidate() {
    d->isValid = false;
}

/** ***************************************************************************/
void Core::Query::setQueryHandlers(const set<QueryHandler *> &queryHandlers) {

    if (d->state != State::Idle)
        return;

    for ( auto handler : queryHandlers )
        if ( handler->isLongRunning() )
            d->asyncHandlers.insert(handler);
        else
            d->syncHandlers.insert(handler);
}


/** ***************************************************************************/
void Core::Query::setFallbacks(const vector<shared_ptr<Core::Item> > &fallbacks) {

    if (d->state != State::Idle)
        return;

     d->fallbacks = fallbacks;
}


/** ***************************************************************************/
void Core::Query::run() {

    if (d->state != State::Idle)
        return;

    d->state = State::Running;


    d->run();
}
