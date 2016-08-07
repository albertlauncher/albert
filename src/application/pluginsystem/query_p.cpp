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

#include <QVariant>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include "abstractextension.h"
#include "abstractitem.h"
#include "albertapp.h"
#include "query_p.h"

namespace {

    struct MatchComparator {
        inline bool operator() (const pair<SharedItem, short>& lhs,
                                const pair<SharedItem, short>& rhs) {
            return lhs.first->urgency() > rhs.first->urgency() // Urgency, for e.g. notifications, Warnings
                    || lhs.first->usageCount() > rhs.first->usageCount() // usage count
                    || lhs.second > rhs.second; // percentual match of the query against the item
        }
    };

}

vector<QString> QueryPrivate::fallbackOrder;

/** ***************************************************************************/
QueryPrivate::Initializer::Initializer(){
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString path = QDir(cacheDir).filePath("fallbackOrder.txt");
    QFile inputFile(path);
    if (inputFile.open(QIODevice::ReadOnly)) {
       QTextStream in(&inputFile);
       while (!in.atEnd()){
           QString line = in.readLine();
           if (!line.isEmpty())
               fallbackOrder.emplace_back(line);
       }
       inputFile.close();
    }
}



/** ***************************************************************************/
QueryPrivate::QueryPrivate(const QString &query, const set<AbstractExtension *> &extensions)
    : searchTerm_(query),
      isValid_(true),
      isRunning_(true),
      showFallbacks_(false),
      mutex_(QMutex::Recursive) {

    Q_ASSERT(!extensions.empty());

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

    // Sort fallbacks
//    sortFallbacks();


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
        connect(fw, &QFutureWatcher<void>::finished, this, &QueryPrivate::onHandlerFinished);
        fw->setFuture(QtConcurrent::run(queryHandler, &AbstractExtension::handleQuery, Query(this)));
        futureWatchers_.push_back(fw);
    }

    emit started();

    UXTimeOut_.setInterval(100);
    UXTimeOut_.setSingleShot(true);
    connect(&UXTimeOut_, &QTimer::timeout, this, &QueryPrivate::onUXTimeOut);
    UXTimeOut_.start();
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
    sortResults();
    emit resultyReady(this);
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
            sortResults();
            emit resultyReady(this);
        }
        if (matches_.size()==0) {
            beginResetModel();
            showFallbacks_ = true;
            endResetModel();
        }
        isRunning_=false;
        emit finished();
    }
}



/** ***************************************************************************/
void QueryPrivate::sortFallbacks()
{

    // Custom sort algorithm, O(n²) but does not matter since size is << 100

    auto swapIt = fallbacks_.begin();
    decltype(swapIt) findIt;
    auto orderIt  = fallbackOrder.begin();

    while (true) {

        // Everything sorted, break
        if (swapIt == fallbacks_.end())
            goto BREAK_LOOP;

        // Remaining fallbacks have never been arranged. Store the ids an quit
        if (orderIt == fallbackOrder.end()) {
            while (swapIt != fallbacks_.end()) {
                fallbackOrder.push_back((*swapIt)->id());
                goto BREAK_LOOP;
            }
        }

        // Check if
        findIt = std::find_if(swapIt+1, fallbacks_.end(), [&orderIt](const SharedItem &item){
            return *orderIt == item->id();
        });


        if (findIt == fallbacks_.end())
            std::swap(*swapIt++, *findIt);
        ++orderIt;
    }

    BREAK_LOOP:;
}



/** ***************************************************************************/
void QueryPrivate::sortResults() {
    mutex_.lock();
    beginResetModel();
    std::sort(matches_.begin(), matches_.end(), MatchComparator());
    endResetModel();
    mutex_.unlock();
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
        const SharedItem &item = showFallbacks_ ? fallbacks_[index.row()] : matches_[index.row()].first;
        switch (role) {
        case Qt::DisplayRole:
            return item->text();
        case Qt::ToolTipRole:
            return item->subtext();
        case Qt::DecorationRole:
            return item->iconPath();
        case Qt::UserRole: {
            QStringList actionTexts;
            for (SharedAction &action : item->actions())
                actionTexts.append(action->text());
            return actionTexts;
        }
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
        const SharedItem &item = showFallbacks_ ? fallbacks_[index.row()] : matches_[index.row()].first;
        mutex_.unlock();
        switch (role) {
        case Qt::UserRole: {

            int actionValue = value.toInt();
            ExecutionFlags flags;

            if (0 <= actionValue && actionValue < static_cast<int>(item->actions().size()))
                item->actions()[actionValue]->activate(&flags);
            else
                item->activate(&flags);

            if (flags.hideWidget)
                qApp->hideWidget();

            if (flags.clearInput)
                qApp->clearInput();

            return true;
        }
        default:
            return false;
        }
    }
    return false;
}
