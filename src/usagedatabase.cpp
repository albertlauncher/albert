// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "include/albert/util/timeprinter.hpp"
#include "usagedatabase.h"
#include <QDir>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
using namespace std;


static const char* db_name = "usagehistory";

Activation::Activation(QString q, QString e, QString i, QString a):
    query(std::move(q)),extension_id(std::move(e)),item_id(std::move(i)),action_id(std::move(a)){}

void UsageDatabase::initializeDatabase()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE", db_name);
    if (!db.isValid())
        qFatal("No sqlite available");

    if (!db.driver()->hasFeature(QSqlDriver::Transactions))
        qFatal("QSqlDriver::Transactions not available.");

    db.setDatabaseName(QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).filePath("albert.db"));

    if (!db.open())
        qFatal("Unable to establish a database connection.");

    DEBG << "Initializing database…";
    QSqlQuery sql(db);
    sql.exec("CREATE TABLE IF NOT EXISTS activation ( "
             "    timestamp INTEGER DEFAULT CURRENT_TIMESTAMP, "
             "    query TEXT, "
             "    extension_id, "
             "    item_id TEXT, "
             "    action_id TEXT "
             "); ");
    if (!sql.isActive())
        qFatal("Unable to create table 'activation': %s", sql.lastError().text().toUtf8().constData());
}

void UsageDatabase::clearActivations()
{
    QSqlDatabase::database(db_name).exec("DROP TABLE activation;");
    initializeDatabase();
}

void UsageDatabase::addActivation(const QString &q, const QString &e, const QString &i, const QString &a)
{
    albert::TimePrinter tp("UsageHistory::addActivation %1 µs");

    QSqlQuery sql(QSqlDatabase::database(db_name));
    sql.prepare("INSERT INTO activation (query, extension_id, item_id, action_id) "
                "VALUES (:query, :extension_id, :item_id, :action_id);");
    sql.bindValue(":query", q);
    sql.bindValue(":extension_id", e);
    sql.bindValue(":item_id", i);
    sql.bindValue(":action_id", a);
    if (!sql.exec())
        qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));
}

std::vector<Activation> UsageDatabase::activations()
{
    albert::TimePrinter tp("UsageHistory::mruScores %1 µs");

    QSqlQuery sql(QSqlDatabase::database(db_name));
    sql.exec("SELECT query, extension_id, item_id, action_id "
             "FROM activation WHERE item_id<>''");

    if (!sql.isActive())
        qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));

    std::vector<Activation> activations;
    while (sql.next())
        activations.emplace_back(sql.value(0).toString(), sql.value(1).toString(),
                                 sql.value(2).toString(), sql.value(3).toString());
    return activations;
}