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

#pragma once
#include <QAbstractListModel>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <set>
#include <map>
#include <vector>
#include <utility>
#include <memory>
#include "query.h"

namespace Core {

class QueryHandler;
class FallbackProvider;
class Extension;
class Item;

/**
 * @brief The QueryExecution class
 * Represents the execution of a query
 */
class QueryExecution : public QAbstractListModel
{
    Q_OBJECT

public:

    enum class State { Idle, Running, Finished };

    QueryExecution(const std::set<QueryHandler*> &,
                   const std::set<FallbackProvider*> &,
                   const QString &queryString,
                   bool fetchIncrementally);
    ~QueryExecution();

    const State &state() const;

    const Query *query();

    void run();
    void cancel();

    // Model interface
    int rowCount(const QModelIndex &) const override;
    QHash<int,QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool canFetchMore(const QModelIndex &) const override;
    void fetchMore(const QModelIndex &) override;

    const std::map<QString, uint> &runtimes();

private:

    void setState(State state);

    void runBatchHandlers();
    void onBatchHandlersFinished();
    void runRealtimeHandlers();
    void onRealtimeHandlersFinsished();
    void insertPendingResults();

    Query query_;
    State state_;

    std::set<QueryHandler*> batchHandlers_;
    std::set<QueryHandler*> realtimeHandlers_;

    mutable std::vector<std::pair<std::shared_ptr<Item>, uint>> results_;
    mutable std::vector<std::pair<std::shared_ptr<Item>, uint>> fallbacks_;
    mutable int sortedItems_ = 0;
    bool fetchIncrementally_ = false;

    QTimer fiftyMsTimer_;
    std::map<QString,uint> runtimes_;

    QFuture<std::pair<QueryHandler*,uint>> future_;
    QFutureWatcher<std::pair<QueryHandler*,uint>> futureWatcher_;

signals:

    void resultsReady(QAbstractItemModel*);
    void stateChanged(State state);

};

}


