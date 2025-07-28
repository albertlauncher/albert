// Copyright (c) 2022-2025 Manuel Schneider

#include "albert.h"
#include "logging.h"
#include "usagedatabase.h"
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
using namespace std;

static const char* db_conn_name = "usagehistory";
static const char* db_file_name = "albert.db";

UsageDatabase &UsageDatabase::instance()
{
    static UsageDatabase usage_database;
    return usage_database;
}

UsageDatabase::UsageDatabase()
{
    DEBG << "Connecting usage database…";

    if (auto db = QSqlDatabase::addDatabase("QSQLITE", db_conn_name);
        !db.isValid())
        qFatal("No sqlite available");

    else if (!db.driver()->hasFeature(QSqlDriver::Transactions))
        qFatal("QSqlDriver::Transactions not available.");

    else if (db.setDatabaseName(QDir(albert::dataLocation()).filePath(db_file_name));
             !db.open())
        qFatal("Database: Unable to establish connection: %s", qPrintable(db.lastError().text()));

    DEBG << "Initializing usage database…";

    QSqlQuery sql(QSqlDatabase::database(db_conn_name));
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

map<QString, uint> UsageDatabase::extensionActivationsSince(const QDateTime &datetime) const
{
    QSqlQuery sql(QSqlDatabase::database(db_conn_name));
    sql.exec(QString("SELECT extension_id, COUNT(extension_id) "
                     "FROM activation "
                     "WHERE timestamp > '%1' "
                     "GROUP BY extension_id").arg(datetime.toString("yyyy-MM-dd hh:mm:ss")));

    if (!sql.isActive())
        qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));

    map<QString, uint> activations;
    while (sql.next())
        activations.emplace(sql.value(0).toString(), sql.value(1).toUInt());

    return activations;
}

void UsageDatabase::addActivation(const QString &q, const QString &e, const QString &i, const QString &a) const
{
    DEBG << "Update usage database…";

    QSqlQuery sql(QSqlDatabase::database(db_conn_name));
    sql.prepare("INSERT INTO activation (query, extension_id, item_id, action_id) "
                "VALUES (:query, :extension_id, :item_id, :action_id);");
    sql.bindValue(":query", q);
    sql.bindValue(":extension_id", e);
    sql.bindValue(":item_id", i);
    sql.bindValue(":action_id", a);
    if (!sql.exec())
        qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));
}

std::unordered_map<ItemKey, double> UsageDatabase::itemUsageScores(double memory_decay) const
{
    DEBG << "Fetching usage scores…";

    struct Activation
    {
        QString query;
        QString extension_id;
        QString item_id;
        QString action_id;
    };
    vector<Activation> activations;

    // Get activations
    QSqlQuery sql(QSqlDatabase::database(db_conn_name));
    sql.exec("SELECT query, extension_id, item_id, action_id FROM activation WHERE item_id<>''");
    if (!sql.isActive())
        qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));
    while (sql.next())
        activations.emplace_back(sql.value(0).toString(), sql.value(1).toString(),
                                 sql.value(2).toString(), sql.value(3).toString());

    // Compute usage weights
    unordered_map<ItemKey, double> usage_weights;
    for (int i = 0, k = (int)activations.size(); i < (int)activations.size(); ++i, --k)
    {
        auto activation = activations[i];
        double weight = pow(memory_decay, k);
        if (const auto &[it, success] = usage_weights.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(activation.extension_id, activation.item_id),
                std::forward_as_tuple(weight));
            !success)
            it->second += weight;
    }

    // Invert the list. Results in ordered by rank map
    map<double, vector<ItemKey>> weight_items;
    for (const auto &[ids, weight] : usage_weights)
        weight_items[weight].emplace_back(ids);

    // Distribute scores linearly over the interval preserving the order
    unordered_map<ItemKey, double> usage_scores;
    double rank = 0.0;
    for (const auto &[weight, vids] : weight_items)
    {
        double score = rank / weight_items.size();
        for (const auto &ids : vids)
            usage_scores.emplace(ids, score);
        rank += 1.0;
    }

    return usage_scores;
}

void UsageDatabase::clearActivations() const
{
    DEBG << "Clearing usage database…";

    QSqlQuery sql(QSqlDatabase::database(db_conn_name));
    sql.exec("TRUNCATE TABLE activation;");
}
