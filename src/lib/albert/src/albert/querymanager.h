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
#include <QObject>
#include <QAbstractItemModel>
#include <vector>

namespace Core {
class ExtensionManager;
class Query;
}

class QueryManager : public QObject
{
    Q_OBJECT

public:

    explicit QueryManager(Core::ExtensionManager* em, QObject *parent = 0);

    void setupSession();
    void teardownSession();
    void startQuery(const QString &searchTerm);

private:

    Core::ExtensionManager *extensionManager_;
    Core::Query *currentQuery_;
    std::vector<Core::Query*> pastQueries_;

signals:

    void resultsReady(QAbstractItemModel*);
};

