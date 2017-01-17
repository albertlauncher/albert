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
#include <set>
#include <map>
#include <vector>
#include <utility>
#include <memory>
#include "core_globals.h"
#include "queryhandler.h"

class QueryManager;

namespace Core {

class Extension;
class Item;

/**
 * @brief The Query class
 * Represents the execution of a query
 */
class EXPORT_CORE Query : public QObject
{
    Q_OBJECT

    friend class ::QueryManager;
    class QueryPrivate;

public:

    enum class State {
        Idle = 0,
        Running,
        Finished,
        Canceled
    };

    const State &state() const;

    const QString &searchTerm() const;

    bool isValid() const;

    void addMatch(std::shared_ptr<Item> item, short score = 0);
    void addMatches(std::vector<std::pair<std::shared_ptr<Item>,short>>::iterator begin,
                    std::vector<std::pair<std::shared_ptr<Item>,short>>::iterator end);

    std::map<QueryHandler *, long> runtimes();

private:

    Query();
    ~Query();

    void setSearchTerm(const QString &);

    void invalidate();

    void setQueryHandlers(const std::set<QueryHandler*> &);

    void setFallbacks(const std::vector<std::shared_ptr<Item>> &);

    void run();

    std::unique_ptr<QueryPrivate> d;

signals:

    void resultsReady(QAbstractItemModel *);
    void finished();

};

}


