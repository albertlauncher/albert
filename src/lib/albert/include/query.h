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
#include <QFutureWatcher>
#include <QMutex>
#include <QString>
#include <QTimer>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <memory>
#include "core_globals.h"
using std::set;
using std::map;
using std::vector;
using std::pair;
using std::shared_ptr;

namespace Core {

class Extension;
class Item;

class MatchOrder
{
public:

    static void update();
    inline bool operator() (const pair<shared_ptr<Item>, short>& lhs,
                            const pair<shared_ptr<Item>, short>& rhs);

private:

    static map<QString, double> order;
};



class EXPORT_CORE Query final : public QAbstractListModel 
{
    Q_OBJECT

public:

    Query(const QString &query, const set<Extension*> &queryHandlers);

    /**
     * @brief Add matches
     * Adds a match to the results. Score describes a percentual match of the
     * query against the item. 0 beeing no match SHRT_MAX beeing a full match.
     * @param item The item to add
     * @param score Matchfactor of the query against the item
     */
    void addMatch(shared_ptr<Item> item, short score = 0);
    void addMatches(vector<std::pair<shared_ptr<Item>,short>>::iterator begin,
                    vector<std::pair<shared_ptr<Item>,short>>::iterator end);



    /**
     * @brief Returns the search term of this query
     */
    const QString &searchTerm() const { return searchTerm_; }
    bool isRunning() { return isRunning_; }


    /**
     * @brief isValid gets the validity of the query.
     * Running handler will stop long operations and discard their matches,
     * if the query is invalid.
     * @return
     */
    bool isValid() const { return isValid_; }
    void invalidate();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:

    void onUXTimeOut();
    void onHandlerFinished();

    const QString searchTerm_;
    bool isValid_;
    bool isRunning_;
    bool showFallbacks_;

    vector<QFutureWatcher<void>*> futureWatchers_;
    map<Extension*, long int> runtimes_;
    mutable QMutex mutex_;
    QTimer UXTimeOut_;

    vector<pair<shared_ptr<Item>, short>> matches_;
    vector<shared_ptr<Item>> fallbacks_;

signals:

    void resultsReady(QAbstractItemModel *);
    void started();
    void finished();

};

}


