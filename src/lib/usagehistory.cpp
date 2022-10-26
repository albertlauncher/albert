// Copyright (c) 2022 Manuel Schneider

#include "usagehistory.h"
#include "logging.h"
#include "scopedtimeprinter.hpp"
#include <shared_mutex>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QDir>
#include <QSqlError>
#include <QStandardPaths>
using namespace std;

static const char* db_name = "usagehistory";
static QSqlDatabase db;
static shared_mutex rw_lock;
static QStringList input_history;
static map<QString,double> mru_scores;

QStringList UsageHistory::inputHistory()
{
    shared_lock<shared_mutex> read_lock(rw_lock);
    return input_history;
}

std::map<QString,double> UsageHistory::mruScores()
{
    shared_lock<shared_mutex> read_lock(rw_lock);
    return mru_scores;
}

void UsageHistory::initializeDatabase()
{
    DEBG << "Create database and tables if they do not exist.";

    db = QSqlDatabase::addDatabase("QSQLITE", "usagehistory");
    if (!db.isValid())
        qFatal("No sqlite available");

    if (!db.driver()->hasFeature(QSqlDriver::Transactions))
        qFatal("QSqlDriver::Transactions not available.");

//    if (!db.driver()->hasFeature(QSqlDriver::LastInsertId))
//        qFatal("QSqlDriver::LastInsertId not available.");

    db.setDatabaseName(QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).filePath("albert.db"));

    if (!db.open())
        qFatal("Unable to establish a database connection.");

    // Initialize database
    auto query = db.exec("CREATE TABLE IF NOT EXISTS activation ( "
                         "    timestamp INTEGER DEFAULT CURRENT_TIMESTAMP, "
                         "    query TEXT NOT NULL, "  // inclusive trigger?
                         //"    extension_id TEXT NOT NULL, "
                         "    item_id TEXT NOT NULL, "
                         "    action_id TEXT NOT NULL "
                         "); ");
    if (!query.isActive())
        qFatal("Unable to create table 'activation': %s", query.lastError().text().toUtf8().constData());
}

void UsageHistory::clearDatabase()
{
    db.exec("DROP TABLE activation;");
    initializeDatabase();
}

void UsageHistory::updateCache()
{
    ScopedTimePrinter("UsageHistory::updateCache %1 µs");
    unique_lock write_lock(rw_lock);

    // Score of a single usage is 1/(<age_in_days>+1).
    mru_scores.clear();
    auto query = db.exec("SELECT item_id, SUM(1/(julianday('now')-julianday(timestamp, 'unixepoch')+1)) AS score "
                         "FROM activation WHERE item_id<>'' GROUP BY item_id, action_id");
    if (!query.isActive())
        qFatal("Unable fetch MRU scores:\n%s\n%s",
               query.executedQuery().toUtf8().constData(),
               query.lastError().text().toUtf8().constData());
    while (query.next())
        mru_scores.emplace(query.value(0).toString(), query.value(1).toDouble());
        // Todo normalize?
        //double max = query.value(1).toDouble();
        //do {
        //    scores_.emplace(query.value(0).toString(), static_cast<uint>(query.value(1).toDouble()*UINT_MAX/max));
        //} while (query.next());

    input_history.clear();
    query = db.exec("SELECT query, timestamp FROM activation GROUP BY query ORDER BY max(timestamp) DESC;");
    if (!query.isActive())
        qFatal("Unable fetch query history:\n%s\n%s",
               query.executedQuery().toUtf8().constData(),
               query.lastError().text().toUtf8().constData());
    while (query.next())
        input_history.append(query.value(0).toString());
}

void UsageHistory::addActivation(const QString &query, const QString &item_id, const QString &action_id)
{
    ScopedTimePrinter("UsageHistory::addActivation %1 µs");

    QSqlQuery sql(db);
    sql.prepare("INSERT INTO activation (query, item_id, action_id) VALUES (:query, :item_id, :action_id);");
    sql.bindValue(":item_id", query);
    sql.bindValue(":item_id", item_id);
    sql.bindValue(":action_id", action_id);
    if (!sql.exec())
        qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));

    updateCache();
}