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
#include <QObject>
#include <QTimer>

#include <map>
#include <set>
#include <memory>
using std::map;
using std::set;
using std::shared_ptr;

class QAbstractItemModel;
class AbstractItem;
class AbstractExtension;
class QueryPrivate;

class QueryHandler final : public QObject
{
    Q_OBJECT

public:

    QueryHandler();
    ~QueryHandler();

    void startQuery(const QString &searchTerm);

    void setupSession();
    void teardownSession();

    void registerExtension(QObject *);
    void unregisterExtension(QObject *);

private:

    void onUXTimeOut();
    void onQueryFinished(QueryPrivate * qp);
    void updateFallbacks(AbstractExtension * ext);

    set<AbstractExtension*> extensions_;
    set<QueryPrivate*> pastQueries_;
    QueryPrivate* currentQuery_;
    map<QString, set<AbstractExtension*>> triggerExtensions_;
    map<shared_ptr<AbstractItem>, AbstractExtension*> fallbacks_;
    QTimer UXTimeOut_;

signals:

    void newModel(QAbstractItemModel *);
};
