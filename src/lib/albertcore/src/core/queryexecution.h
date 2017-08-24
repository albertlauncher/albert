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
                   const QString &query);
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

    const std::map<QString, uint> &runtimes();

private:

    void setState(State state);

    void runBatchHandlers();
    void onBatchHandlersFinished();
    void runRealitimeHandlers();
    void onRealitimeHandlersFinsished();
    void insertPendingResults();
    void finishQuery();

    Query query_;
    State state_;

    std::set<QueryHandler*> batchHandlers_;
    std::set<QueryHandler*> realtimeHandlers_;

    std::vector<std::shared_ptr<Item>> results_;
    std::vector<std::shared_ptr<Item>> fallbacks_;

    QTimer fiftyMsTimer_;
    std::map<QString,uint> runtimes_;

    QFuture<std::pair<QueryHandler*,uint>> future_;
    QFutureWatcher<std::pair<QueryHandler*,uint>> futureWatcher_;

signals:

    void resultsReady(QAbstractItemModel*);
    void stateChanged(State state);

};

}


