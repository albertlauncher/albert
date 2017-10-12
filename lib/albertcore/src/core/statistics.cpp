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

#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QStandardPaths>
#include "statistics.h"
using namespace Core;
using namespace std;
using namespace std::chrono;

namespace {
char const * const statsDbName = "statisticsDatabase";
}

std::map<QString, unsigned long long> Statistics::handlerIds;
unsigned long long Statistics::lastQueryId;
std::vector<QueryStatistics> Statistics::records;


/** ***********************************************************************************************/
void Statistics::initialize() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", statsDbName);
    if ( !db.isValid() )
        qFatal("No sqlite available");

    if (!db.driver()->hasFeature(QSqlDriver::Transactions))
        qFatal("QSqlDriver::Transactions not available.");

    db.setDatabaseName(QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).filePath("core.db"));
    if (!db.open())
        qFatal("Unable to establish a database connection.");

    db.transaction();

    QSqlQuery q(db);
    if (!q.exec("CREATE TABLE IF NOT EXISTS query_handler ( "
                "  id INTEGER PRIMARY KEY NOT NULL, "
                "  string_id TEXT UNIQUE NOT NULL "
                "); "))
        qFatal("Unable to create table 'query_handler': %s", q.lastError().text().toUtf8().constData());

    if (!q.exec("CREATE TABLE IF NOT EXISTS query ( "
                "    id INTEGER PRIMARY KEY, "
                "    input TEXT NOT NULL, "
                "    cancelled INTEGER NOT NULL, "
                "    runtime INTEGER NOT NULL, "
                "    timestamp INTEGER DEFAULT CURRENT_TIMESTAMP "
                "); "))
        qFatal("Unable to create table 'query': %s", q.lastError().text().toUtf8().constData());

    if (!q.exec("CREATE TABLE IF NOT EXISTS execution ( "
                "    query_id INTEGER NOT NULL REFERENCES query(id) ON UPDATE CASCADE, "
                "    handler_id INTEGER NOT NULL REFERENCES query_handler(id) ON UPDATE CASCADE, "
                "    runtime INTEGER NOT NULL, "
                "    PRIMARY KEY (query_id, handler_id) "
                ") WITHOUT ROWID; "))
        qFatal("Unable to create table 'execution': %s", q.lastError().text().toUtf8().constData());

    if (!q.exec("CREATE TABLE IF NOT EXISTS activation ( "
                "    query_id INTEGER PRIMARY KEY NOT NULL REFERENCES query(id) ON UPDATE CASCADE, "
                "    item_id TEXT NOT NULL "
                "); "))
        qFatal("Unable to create table 'activation': %s", q.lastError().text().toUtf8().constData());

    if (!q.exec("DELETE FROM query "
                "WHERE julianday('now')-julianday(timestamp)>30; "))
        qWarning("Unable to cleanup 'query' table.");

    db.commit();


    // Get last query id
    lastQueryId = 0;
    q.prepare("SELECT MAX(id) FROM query;");
    if (!q.exec())
        qFatal("SQL ERROR: %s %s", qPrintable(q.executedQuery()), qPrintable(q.lastError().text()));
    if (q.next())
        lastQueryId = q.value(0).toULongLong();

    // Get the handlers Ids
    q.exec("SELECT string_id, id FROM query_handler;");
    if (!q.exec())
        qFatal("SQL ERROR: %s %s", qPrintable(q.executedQuery()), qPrintable(q.lastError().text()));
    while(q.next())
        handlerIds.emplace(q.value(0).toString(), q.value(1).toULongLong());

}


/** ***********************************************************************************************/
void Statistics::addRecord(const QueryStatistics &record) {
    records.push_back(record);
}


/** ***********************************************************************************************/
void Statistics::commitRecords() {

    QSqlDatabase db = QSqlDatabase::database(statsDbName);
    QSqlQuery query(db);

    db.transaction();
    for (const QueryStatistics& record : records) {

        ++lastQueryId;

        // Create a query record
        query.prepare("INSERT INTO query (id, input, cancelled, runtime, timestamp) "
                      "VALUES (:id, :input, :cancelled, :runtime, :timestamp);");
        query.bindValue(":id", lastQueryId);
        query.bindValue(":input", record.input);
        query.bindValue(":cancelled", record.cancelled);
        query.bindValue(":runtime", static_cast<qulonglong>(duration_cast<microseconds>(record.end-record.start).count()));
        query.bindValue(":timestamp", static_cast<qulonglong>(duration_cast<seconds>(record.start.time_since_epoch()).count()));
        if (!query.exec())
            qFatal("SQL ERROR: %s", qPrintable(query.lastError().text()));

        // Make sure all handlers exits in database
        query.prepare("INSERT INTO query_handler (string_id) VALUES (:id);");
        for ( auto & runtime : record.runtimes ) {
            auto it = handlerIds.find(runtime.first);
            if ( it == handlerIds.end()){
                query.bindValue(":id", runtime.first);
                if (!query.exec())
                    qFatal("SQL ERROR: %s %s", qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
                handlerIds.emplace(runtime.first, query.lastInsertId().toULongLong());
            }
        }

        // Create execution records
        query.prepare("INSERT INTO execution (query_id, handler_id, runtime) "
                      "VALUES (:query_id, :handler_id, :runtime);");
        for ( auto & runtime : record.runtimes ) {
            query.bindValue(":query_id", lastQueryId);
            query.bindValue(":handler_id", handlerIds[runtime.first]);
            query.bindValue(":runtime", runtime.second);
            if (!query.exec())
                qFatal("SQL ERROR: %s %s", qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
        }

        // Create activation record
        if (!record.activatedItem.isNull()) {
            query.prepare("INSERT INTO activation (query_id, item_id) VALUES (:query_id, :item_id);");
            query.bindValue(":query_id", lastQueryId);
            query.bindValue(":item_id", record.activatedItem);
            if (!query.exec())
                qFatal("SQL ERROR: %s %s", qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
        }
    }
    db.commit();
}


/** ***********************************************************************************************/
void Statistics::clearRecentlyUsed() {
    QSqlQuery("DELETE FROM activation;", QSqlDatabase::database(statsDbName));
}


/** ***********************************************************************************************/
QStringList Statistics::getRecentlyUsed() {
    QStringList mru;
    QSqlQuery query(QSqlDatabase::database(statsDbName));
    query.exec("SELECT input "
               "FROM activation a JOIN  query q ON a.query_id = q.id "
               "GROUP BY input  "
               "ORDER BY max(timestamp) DESC;");
    while (query.next())
        mru.append(query.value(0).toString());
    return mru;
}


/** ************************************************************************************************
 * @brief Core::MatchCompare::update
 * Update the usage score:
 * Score of a single usage is 1/(<age_in_days>+1).
 * Accumulate all scores groupes by itemId.
 * Normalize the scores to the range of UINT_MAX.
 */
std::map<QString, uint> Statistics::getRanking() {
    map<QString, uint> ranking;
    QSqlQuery query(QSqlDatabase::database(statsDbName));
    query.exec("SELECT a.item_id AS id, SUM(1/(julianday('now')-julianday(timestamp, 'unixepoch')+1)) AS score "
               "FROM activation a JOIN  query q ON a.query_id = q.id "
               "WHERE a.item_id<>'' "
               "GROUP BY a.item_id "
               "ORDER BY score DESC");

    if ( !query.next() )
        return map<QString, uint>();

    double max = query.value(1).toDouble();
    do {
        ranking.emplace(query.value(0).toString(), static_cast<uint>(query.value(1).toDouble()*UINT_MAX/max));
    } while (query.next());
    return ranking;
}


