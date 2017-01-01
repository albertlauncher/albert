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

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QtConcurrent/QtConcurrent>
#include <QVariant>
#include <algorithm>
#include <chrono>
#include <map>
#include "action.h"
#include "extension.h"
#include "item.h"
#include "query.h"
using std::chrono::system_clock;
using std::map;



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
map<QString, double> Core::MatchOrder::order;

bool Core::MatchOrder::operator()(const pair<shared_ptr<Item>, short> &lhs,
                                  const pair<shared_ptr<Item>, short> &rhs) {

    // Return true if urgency is higher
    if (lhs.first->urgency() > rhs.first->urgency())
        return true;

    // Find the ids in the usage scores
    const auto &lit = order.find(lhs.first->id());
    const auto &rit = order.find(rhs.first->id());

    // If left side has no entry it cannot be higher continue with relevance
    if (lit==order.cend())
        goto relevance;

    // If left side has an entry bur right side not left side must be higher
    // given the condition that all values in the usage rankings are >0
    if (rit==order.cend())
        return true;

    // Both scores available, return true if lhs is higher
    if (order[lhs.first->id()] > order[rhs.first->id()]) // usage count
        return true;

    // Return true if relevance is higher else false
    relevance:
    return lhs.second > rhs.second; // percentual match of the query against the item
}

void Core::MatchOrder::update() {
    order.clear();

    // Update the results ranking
    QSqlQuery query;
    query.exec("SELECT t.itemId AS id, SUM(t.score) AS usageScore "
               "FROM ( "
               " SELECT itemId, 1/max(julianday('now')-julianday(timestamp),1) AS score from usages "
               ") t "
               "GROUP BY t.itemId");
    while (query.next())
        MatchOrder::order.emplace(query.value(0).toString(),
                                  query.value(1).toDouble());
}


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/

class Results {

};

/** ***************************************************************************/
/** ***************************************************************************/
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
    map<QueryHandler*, long int> runtimes;

    vector<shared_ptr<Item>> results;
    vector<shared_ptr<Item>> fallbacks;

    QTimer fiftyMsTimer;
    mutable QMutex pendingResultsMutex;
    vector<pair<shared_ptr<Item>, short>> pendingResults;

    QFutureWatcher<pair<QueryHandler*,long>> futureWatcher;


    /** ***************************************************************************/
    void onSyncHandlersFinsished() {

        // Get the runtimes
        for ( auto it = futureWatcher.future().begin(); it != futureWatcher.future().end(); ++it )
            runtimes.emplace(it->first, it->second);

        // Lock the pending results
        QMutexLocker lock(&pendingResultsMutex);

        // Sort the results
        std::sort(pendingResults.begin(),
                  pendingResults.end(),
                  MatchOrder());

        // Preallocate space in "results" to avoid multiple allocations
        results.reserve(results.size() + pendingResults.size());

        // Move the items of the "pending results" into "results"
        std::transform(pendingResults.begin(),
                       pendingResults.end(),
                       std::back_inserter(results),
                       [](const pair<shared_ptr<Item>,short>& p){ return std::move(p.first); });

        pendingResults.clear();

        emit q->resultsReady(this);

        // Define the map function
        std::function<pair<QueryHandler*,long>(QueryHandler*)> mapFunction = [this](QueryHandler* queryHandler) {
            system_clock::time_point then = system_clock::now();
            queryHandler->handleQuery(q);
            system_clock::time_point now = system_clock::now();
            return std::make_pair(queryHandler, std::chrono::duration_cast<std::chrono::microseconds>(now-then).count());
        };

        // Call onAsyncHandlersFinsished when async handlers finished
        futureWatcher.disconnect();
        connect(&futureWatcher, &QFutureWatcher<pair<QueryHandler*,long>>::finished,
                std::bind(&QueryPrivate::onAsyncHandlersFinsished, this));

        // Run the handlers concurrently
        futureWatcher.setFuture(QtConcurrent::mapped(asyncHandlers.begin(),
                                                     asyncHandlers.end(),
                                                     mapFunction));

        connect(&fiftyMsTimer, &QTimer::timeout,
                this, &QueryPrivate::insertPendingResults);

        fiftyMsTimer.start(50);

    }


    /** ***************************************************************************/
    void insertPendingResults() {
        /*
         * Get the results
         */

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
    void onAsyncHandlersFinsished() {

        // Finally done
        fiftyMsTimer.stop();
        fiftyMsTimer.disconnect();
        insertPendingResults();

        // If results are empty show fallbacks
        if(results.empty()){
            beginInsertRows(QModelIndex(), 0, fallbacks.size() - 1);
            results.insert(results.end(),
                           fallbacks.begin(),
                           fallbacks.end());
            endInsertRows();
        }

        // Get the runtimes
        for ( auto it = futureWatcher.future().begin(); it != futureWatcher.future().end(); ++it )
            runtimes.emplace(it->first, it->second);

        // Save the runtimes
        QSqlDatabase db = QSqlDatabase::database();
        db.transaction();
        QSqlQuery sqlQuery;
        for ( auto &queryHandlerRuntimeEntry : runtimes ) {
            sqlQuery.prepare("INSERT INTO runtimes (extensionId, runtime) VALUES (:extensionId, :runtime);");
            sqlQuery.bindValue(":extensionId", queryHandlerRuntimeEntry.first->id);
            sqlQuery.bindValue(":runtime", static_cast<qulonglong>(queryHandlerRuntimeEntry.second));
            if (!sqlQuery.exec())
                qWarning() << sqlQuery.lastError();
        }
        db.commit();

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
            case Qt::UserRole+101: // AltAction
                if (0U < fallbacks.size() && 0U < item->actions().size()) {
                    fallbacks[0]->actions()[0]->activate();
                    itemId = fallbacks[0]->id();
                }
                break;
            case Qt::UserRole+102: // MetaAction
                if (1U < item->actions().size())
                    item->actions()[1]->activate();
                break;
            case Qt::UserRole+103: // ControlAction
                if (2U < item->actions().size())
                    item->actions()[2]->activate();
                break;
            case Qt::UserRole+104: // ShiftAction
                if (3U < item->actions().size())
                    item->actions()[3]->activate();
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
Core::Query::Query() : d(new QueryPrivate(this)) {}


/** ***************************************************************************/
Core::Query::~Query() { delete d; }


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
                                 make_move_iterator(begin),
                                 make_move_iterator(end));
        d->pendingResultsMutex.unlock();
    }
}


/** ***************************************************************************/
void Core::Query::addMatches(set<pair<shared_ptr<Item>,short>>::iterator begin,
                             set<pair<shared_ptr<Item>,short>>::iterator end) {
    if ( d->isValid ) {
        d->pendingResultsMutex.lock();
        d->pendingResults.insert(d->pendingResults.end(),
                                 make_move_iterator(begin),
                                 make_move_iterator(end));
        d->pendingResultsMutex.unlock();
    }
}


/** ***************************************************************************/
void Core::Query::addMatches(map<shared_ptr<Item>,short>::iterator begin,
                             map<shared_ptr<Item>,short>::iterator end) {
    if ( d->isValid ) {
        d->pendingResultsMutex.lock();
        d->pendingResults.insert(d->pendingResults.end(),
                                 make_move_iterator(begin),
                                 make_move_iterator(end));
        d->pendingResultsMutex.unlock();
    }
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

    // Define the map function
    std::function<pair<QueryHandler*,long>(QueryHandler*)> mapFunction = [this](QueryHandler* queryHandler) {
        system_clock::time_point then = system_clock::now();
        queryHandler->handleQuery(this);
        system_clock::time_point now = system_clock::now();
        return std::make_pair(queryHandler, std::chrono::duration_cast<std::chrono::microseconds>(now-then).count());
    };

    // Call onMapFinshed when all handlers finished
    connect(&d->futureWatcher, &QFutureWatcher<pair<QueryHandler*,long>>::finished,
            std::bind(&QueryPrivate::onSyncHandlersFinsished, d));

    // Run the handlers concurrently
    d->futureWatcher.setFuture(QtConcurrent::mapped(d->syncHandlers.begin(),
                                                    d->syncHandlers.end(),
                                                    mapFunction));
}
