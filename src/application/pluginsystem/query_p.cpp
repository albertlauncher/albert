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
#include "abstractaction.h"
#include "abstractextension.h"
#include "abstractitem.h"
#include "query_p.h"
using std::chrono::system_clock;
using std::map;


/** ***************************************************************************/
map<QString, double> MatchOrder::order;

bool MatchOrder::operator()(const pair<SharedItem, short> &lhs, const pair<SharedItem, short> &rhs) {

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

void MatchOrder::update() {
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
/** ***************************************************************************/
QueryPrivate::QueryPrivate(const QString &query, const set<AbstractExtension *> &extensions)
    : searchTerm_(query),
      isValid_(true),
      isRunning_(true),
      showFallbacks_(false),
      mutex_(QMutex::Recursive) {

    Q_ASSERT(!extensions.empty());

    /* Start multithreaded and asynchronous computation of results */

    // Check if some queries want to be runned by trigger
    vector<AbstractExtension*> queryHandlers;
    for ( AbstractExtension *ext : extensions)
        if ( ext->runExclusive() )
            for ( QString triggerPrefix : ext->triggers() )
                if ( searchTerm_.startsWith(triggerPrefix) )
                    queryHandlers.push_back(ext);

    if (queryHandlers.empty())
        for ( AbstractExtension *ext : extensions)
            if ( !ext->runExclusive() )
                queryHandlers.push_back(ext);

    // Start handlers
    for (AbstractExtension *queryHandler : queryHandlers) {
        QFutureWatcher<void>* fw = new QFutureWatcher<void>(this);
        system_clock::time_point start = system_clock::now();
        connect(fw, &QFutureWatcher<void>::finished, [queryHandler, start, this](){
            runtimes_.emplace(queryHandler, std::chrono::duration_cast<std::chrono::microseconds>(system_clock::now()-start).count());
            onHandlerFinished();
        });
        fw->setFuture(QtConcurrent::run(queryHandler, &AbstractExtension::handleQuery, Query(this)));
        futureWatchers_.push_back(fw);
    }

    emit started();

    UXTimeOut_.setInterval(100);
    UXTimeOut_.setSingleShot(true);
    connect(&UXTimeOut_, &QTimer::timeout, this, &QueryPrivate::onUXTimeOut);
    UXTimeOut_.start();

    /* Get fallbacks */

    // Request the fallbacks multithreaded
    QFutureSynchronizer<vector<SharedItem>> synchronizer;
    for ( AbstractExtension *ext : extensions)
        synchronizer.addFuture(QtConcurrent::run(ext, &AbstractExtension::fallbacks, searchTerm_));
    synchronizer.waitForFinished();

    // Get fallbacks
    for (const QFuture<vector<SharedItem>> &future : synchronizer.futures())
        for (SharedItem &item : future.result())
            fallbacks_.push_back(item);
}



/** ***************************************************************************/
void QueryPrivate::invalidate() {
    isValid_ = false;
}



/** ***************************************************************************/
void QueryPrivate::addMatch(shared_ptr<AbstractItem> item, short score) {
    if ( isValid_ ) {
        mutex_.lock();
        beginInsertRows(QModelIndex(), matches_.size(), matches_.size());
        matches_.push_back({item, score});
        endInsertRows();
        mutex_.unlock();
    }
}



/** ***************************************************************************/
void QueryPrivate::addMatches(vector<std::pair<SharedItem,short>>::iterator begin,
                              vector<std::pair<SharedItem,short>>::iterator end) {
    if ( isValid_ ) {
        mutex_.lock();
        beginInsertRows(QModelIndex(), matches_.size(), matches_.size() + std::distance(begin, end));
        matches_.insert(matches_.end(), begin, end);
        endInsertRows();
        mutex_.unlock();
    }
}



/** ***************************************************************************/
void QueryPrivate::onUXTimeOut() {
    mutex_.lock();
    std::sort(matches_.begin(), matches_.end(), MatchOrder());
    mutex_.unlock();
    emit resultsReady(this);
}



/** ***************************************************************************/
void QueryPrivate::onHandlerFinished(){
    // Emit finished if all are finished
    bool fin = true;
    for (QFutureWatcher<void> const  * const futureWatcher : futureWatchers_)
        fin &= futureWatcher->isFinished();
    if ( fin ) {
        /*
         * If the query finished before the UX timeout timed out everything is
         * fine. If not (UXTimeOut timer still active) the results have already
         * been sorted and published. The user sees a list that must not be
         * rearranged for better user experience.
         */
        if (UXTimeOut_.isActive()){
            UXTimeOut_.stop();
            onUXTimeOut();
        }
        if (matches_.size()==0) {
            beginResetModel();
            showFallbacks_ = true;
            endResetModel();
        }

        // Save runtimes
        if (isValid_){ // Dont count cancelled queries
            QSqlDatabase db = QSqlDatabase::database();
            db.transaction();
            QSqlQuery sqlQuery;
            for (auto &e : runtimes_) {
                sqlQuery.prepare("INSERT INTO runtimes (extensionId, runtime) VALUES (:extensionId, :runtime);");
                sqlQuery.bindValue(":extensionId", e.first->id);
                sqlQuery.bindValue(":runtime", static_cast<qulonglong>(e.second));
                if (!sqlQuery.exec())
                    qWarning() << sqlQuery.lastError();
            }
            db.commit();
        }

        isRunning_=false;
        emit finished();
    }
}


/** ***************************************************************************/
int QueryPrivate::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    QMutexLocker m(&mutex_);
    return static_cast<int>(showFallbacks_ ? fallbacks_.size() : matches_.size());
}



/** ***************************************************************************/
QVariant QueryPrivate::data(const QModelIndex &index, int role) const {
    if (index.isValid()) {
        SharedItem item = showFallbacks_
                ? fallbacks_[static_cast<size_t>(index.row())]
                : matches_[static_cast<size_t>(index.row())].first;

        switch (role) {
        case Qt::DisplayRole:
            return item->text();
        case Qt::ToolTipRole:
            return item->subtext();
        case Qt::DecorationRole:
            return item->iconPath();

        case Qt::UserRole: { // Actions list
            QStringList actionTexts;
            for (const SharedAction &action : item->actions())
                actionTexts.append(action->text());
            return actionTexts;
        }

        case Qt::UserRole+100: // DefaultAction
            return (0 < static_cast<int>(item->actions().size())) ? item->actions()[0]->text() : item->subtext();
        case Qt::UserRole+101: // AltAction
            return "Search '"+searchTerm_+"' using default fallback";
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
bool QueryPrivate::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid()) {
        mutex_.lock();
        SharedItem item = showFallbacks_
                ? fallbacks_[static_cast<size_t>(index.row())]
                : matches_[static_cast<size_t>(index.row())].first;
        mutex_.unlock();
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
            if (0U < fallbacks_.size() && 0U < item->actions().size()) {
                item = fallbacks_[0];
                item->actions()[0]->activate();
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
        if (isValid_){ // Dont count cancelled queries
            QSqlQuery query;
            query.prepare("INSERT INTO usages (input, itemId) VALUES (:input, :itemId);");
            query.bindValue(":input", searchTerm_);
            query.bindValue(":itemId", item->id());
            if (!query.exec())
                qWarning() << query.lastError();
        }
    }
    return false;
}
