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
#include <vector>
#include <memory>
using std::map;
using std::vector;
using std::set;
using std::shared_ptr;

class QAbstractItemModel;
class AlbertItem;
class IExtension;
class QueryPrivate;

class ExtensionManager final : public QObject
{
    Q_OBJECT

public:

    ExtensionManager();
    ~ExtensionManager();

    void startQuery(const QString &searchTerm);

    void setupSession();
    void teardownSession();

    void registerExtension(QObject *);
    void unregisterExtension(QObject *);

private:

    void onUXTimeOut();
    void onQueryFinished(QueryPrivate * qp);
    void updateFallbacks();

    set<IExtension*> extensions_;
    set<QueryPrivate*> pastQueries_;
    QueryPrivate* currentQuery_;
    map<QString, set<IExtension*>> triggerExtensions_;
    vector<shared_ptr<AlbertItem>> fallbacks_;
    QTimer UXTimeOut_;

signals:

    void newModel(QAbstractItemModel *);
};
