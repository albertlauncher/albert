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

#pragma once
#include <QAbstractListModel>
#include <QDebug>
#include <QVariant>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QMutexLocker>

#include <algorithm>
#include <vector>
#include <memory>
using std::shared_ptr;
using std::vector;

#include "abstractextension.h"
#include "abstractitem.h"
#include "albertapp.h"


struct Match {
    SharedItem item;
    short score;
};



/** ***************************************************************************/
class QueryPrivate : public QAbstractListModel
{

    Q_OBJECT

public:

    enum class State { Initial, Running, Finished };

    QueryPrivate(const QString &query, const QString &trigger = QString()) :
        searchTerm_(query),
        trigger_(trigger),
        isValid_(true),
        state_(State::Initial),
        mutex_(QMutex::Recursive)
    { }



    /** ***********************************************************************/
    void start() {
        if (isValid_) {
            // Start handlers
            for (AbstractExtension *extension : queryHandlers_) {
                QFutureWatcher<void>* fw = new QFutureWatcher<void>(this);
                connect(fw, &QFutureWatcher<void>::finished, this, &QueryPrivate::onHandlerFinished);
                fw->setFuture(QtConcurrent::run(extension, &AbstractExtension::handleQuery, Query(this)));
                futureWatchers_.push_back(fw);
            }
            state_ = State::Running;
        }
    }



    /** ***********************************************************************/
    void stop() {
        isValid_ = false;
        if ( state_ == State::Initial)
            state_ = State::Finished;
        else
            for (QFutureWatcher<void>* futureWatcher : futureWatchers_)
                futureWatcher->disconnect(this);
    }



    /** ***********************************************************************/
    void addMatch(Match match) {
        if ( isValid_ ) {
            mutex_.lock();
            beginInsertRows(QModelIndex(), matches_.size(), matches_.size());
            matches_.push_back(match);
            endInsertRows();
            mutex_.unlock();
        }
    }



    /** ***********************************************************************/
    void addMatch(shared_ptr<AbstractItem> item, short score = 0) {
        if ( isValid_ ) {
            mutex_.lock();
            beginInsertRows(QModelIndex(), matches_.size(), matches_.size());
            matches_.push_back({item, score});
            endInsertRows();
            mutex_.unlock();
        }
    }



    /** ***********************************************************************/
    void addMatches(vector<Match>::iterator begin, vector<Match>::iterator end) {
        if ( isValid_ ) {
            mutex_.lock();
            beginInsertRows(QModelIndex(), matches_.size(), matches_.size() + std::distance(begin, end));
            std::copy(begin, end, std::back_inserter(matches_));
            endInsertRows();
            mutex_.unlock();
        }
    }



    /** ***********************************************************************/
    void addHandler(AbstractExtension *handler) {
        if ( state_ == State::Initial )
            queryHandlers_.push_back(handler);
    }



    /** ***********************************************************************/
    vector<AbstractExtension *> handlers() {
        return queryHandlers_;
    }


    void invalidate() { isValid_ = false; }
    bool isValid() { return isValid_; }

    const QString &searchTerm() const { return searchTerm_; }
    const QString &trigger() const { return trigger_; }
    State state() const { return state_; }



    /** ***********************************************************************/
    /** ***********************************************************************/
    /** ***********************************************************************/
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        QMutexLocker m(&mutex_);
        return static_cast<int>(matches_.size());
    }



    /** ***********************************************************************/
    QVariant data(const QModelIndex & index, int role) const override {
        if (index.isValid()) {
            mutex_.lock();
            SharedItem item = matches_[index.row()].item;
            mutex_.unlock();
            switch (role) {
            case Qt::DisplayRole:
                return item->text(searchTerm_);
            case Qt::ToolTipRole:
                return item->subtext(searchTerm_);
            case Qt::DecorationRole:
                return item->iconPath();
            case Qt::UserRole: {
                QStringList actionTexts;
                for (SharedAction &action : item->actions())
                    actionTexts.append(action->text(searchTerm_));
                return actionTexts;
            }
            default:
                return QVariant();
            }
        }
        return QVariant();
    }



    /** ***********************************************************************/
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (index.isValid()) {
            mutex_.lock();
            SharedItem item = matches_[index.row()].item;
            mutex_.unlock();
            switch (role) {
            case Qt::UserRole: {

                int actionValue = value.toInt();
                ExecutionFlags flags;

                if (0 <= actionValue && actionValue < static_cast<int>(item->actions().size()))
                    item->actions()[actionValue]->activate(&flags, searchTerm_);
                else
                    item->activate(&flags, searchTerm_);

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



    /** ***********************************************************************/
    void sort() {
        mutex_.lock();
        beginResetModel();
        std::sort(matches_.begin(), matches_.end(), MatchComparator());
        endResetModel();
        mutex_.unlock();
    }

protected:

    /** ***********************************************************************/
    void onHandlerFinished(){
        // Emit finished if all are finished
        bool fin = true;
        for (QFutureWatcher<void> const  * const futureWatcher : futureWatchers_)
            fin &= futureWatcher->isFinished();
        if ( fin ) {
            state_ = State::Finished;
            emit finished(this);
        }
    }


    /** ***********************************************************************/
    struct MatchComparator {
        inline bool operator() (const Match& lhs, const Match& rhs) {
            return lhs.item->urgency() > rhs.item->urgency() // Urgency, for e.g. notifications, Warnings
                    || lhs.item->usageCount() > rhs.item->usageCount() // usage count
                    || lhs.score > rhs.score; // percentual match of the query against the item
        }
    };

    const QString searchTerm_;
    const QString trigger_;
    bool isValid_;
    State state_;
    vector<AbstractExtension*> queryHandlers_;
    vector<QFutureWatcher<void>*> futureWatchers_;
    vector<Match> matches_;
    mutable QMutex mutex_;

signals:

    void finished(QueryPrivate *);
};


