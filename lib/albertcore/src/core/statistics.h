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
#include <QStringList>
#include <QAbstractItemModel>
#include <map>
#include <chrono>

namespace Core {

struct QueryStatistics {
    QString input;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    std::map<QString, uint> runtimes;
    bool cancelled = false;
    QString activatedItem;
};

class Statistics final
{
public:

    static void initialize();
    static void clearRecentlyUsed();
    static QStringList getRecentlyUsed();
    static std::map<QString, uint> getRanking();
    static void addRecord(const QueryStatistics&);
    static void commitRecords();

private:

    static std::map<QString, unsigned long long> handlerIds;
    static unsigned long long lastQueryId;
    static std::vector<QueryStatistics> records;
};

}
