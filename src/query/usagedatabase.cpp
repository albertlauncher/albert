// Copyright (c) 2022-2024 Manuel Schneider

#include "albert.h"
#include "extension.h"
#include "logging.h"
#include "rankitem.h"
#include "usagedatabase.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <mutex>
#include <shared_mutex>
using namespace albert;
using namespace std;

static const char* db_conn_name = "usagehistory";
static const char* db_file_name = "albert.db";
static const char*  CFG_MEMORY_DECAY = "memoryDecay";
static const double DEF_MEMORY_DECAY = 0.5;
static const char*  CFG_PRIO_PERFECT = "prioritizePerfectMatch";
static const bool   DEF_PRIO_PERFECT = true;

// Hashing specialization for Key
template <>
struct std::hash<Key>
{
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key#comment39936543_17017281
    inline std::size_t operator()(const Key& k) const
    { return (qHash(k.first) ^ (qHash(k.second)<< 1)); }
};

shared_mutex UsageHistory::global_data_mutex_;
UsageScores UsageHistory::usage_scores_;
bool UsageHistory::prioritize_perfect_match_;
double UsageHistory::memory_decay_;
recursive_mutex UsageHistory::db_recursive_mutex_;

Activation::Activation(QString q, QString e, QString i, QString a):
    query(::move(q)),extension_id(::move(e)),item_id(::move(i)),action_id(::move(a)){}

void UsageHistory::initialize()
{
    db_connect();
    db_initialize();

    auto s = settings();
    memory_decay_ = s->value(CFG_MEMORY_DECAY, DEF_MEMORY_DECAY).toDouble();
    prioritize_perfect_match_ = s->value(CFG_PRIO_PERFECT, DEF_PRIO_PERFECT).toBool();

    updateScores();
}

void UsageHistory::applyScore(const QString &extension_id, RankItem *rank_item)
{
    /*
     *  p  r     | ( 3, 4] |  3 + mru_score      | prioritized recent perfect matches
     *  p !r     | ( 2, 3] |  2 + 1 / text_len   | prioritized perfect matches
     * !p  r     | ( 1, 2] |  1 + mru_score      | recent matches
     * !p !r  m  | ( 0, 1] |  match_score        | matches
     * !p !r !m  | (-1, 0] |  -1 + 1 / text_len  | no match
     */

    const Key key(extension_id, rank_item->item->id());

    if (prioritize_perfect_match_ && rank_item->score == 1.0f)
    {
        if (const auto &it = usage_scores_.find(key); it != usage_scores_.end())
            rank_item->score = 3.0f + it->second;
        else
            rank_item->score = 2.0f + 1.0f / rank_item->item->text().length();
    }
    else
    {
        if (const auto &it = usage_scores_.find(key); it != usage_scores_.end())
            rank_item->score = 1.0f + it->second;
        else if (rank_item->score == 0.0f)
            rank_item->score = -1.0f + 1.0f / rank_item->item->text().length();
        // else score remains unmodified
    }
}

void UsageHistory::applyScores(const QString &id, vector<RankItem> &rank_items)
{
    shared_lock lock(global_data_mutex_);
    for (auto &rank_item : rank_items)
        applyScore(id, &rank_item);
}

void UsageHistory::applyScores(vector<pair<Extension *, RankItem>> *rank_items)
{
    shared_lock lock(global_data_mutex_);
    for (auto &[extension, rank_item] : *rank_items)
        applyScore(extension->id(), &rank_item);
}

double UsageHistory::memoryDecay()
{
    shared_lock lock(global_data_mutex_);
    return memory_decay_;
}

void UsageHistory::setMemoryDecay(double value)
{
    settings()->setValue(CFG_MEMORY_DECAY, value);

    global_data_mutex_.lock();
    memory_decay_ = value;
    global_data_mutex_.unlock();

    updateScores();
}

bool UsageHistory::prioritizePerfectMatch()
{
    shared_lock lock(global_data_mutex_);
    return prioritize_perfect_match_;
}

void UsageHistory::setPrioritizePerfectMatch(bool value)
{
    settings()->setValue(CFG_PRIO_PERFECT, value);
    unique_lock lock(global_data_mutex_);
    prioritize_perfect_match_ = value;
}

void UsageHistory::addActivation(const QString &qid, const QString &eid,
                                 const QString &iid, const QString &aid)
{
    db_addActivation(qid, eid, iid, aid);
    updateScores();
}

map<QString, uint> UsageHistory::activationsSince(const QDateTime &datetime)
{
    unique_lock lock(db_recursive_mutex_);

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

void UsageHistory::updateScores()
{
    DEBG << "Updating usage scores…";
    unique_lock lock(db_recursive_mutex_);

    // Get activations
    QSqlQuery sql(QSqlDatabase::database(db_conn_name));
    vector<Activation> activations;
    sql.exec("SELECT query, extension_id, item_id, action_id FROM activation WHERE item_id<>''");
    if (!sql.isActive())
        qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));
    while (sql.next())
        activations.emplace_back(sql.value(0).toString(), sql.value(1).toString(),
                                 sql.value(2).toString(), sql.value(3).toString());

    // Compute usage weights
    UsageScores usage_weights;
    for (int i = 0, k = (int)activations.size(); i < (int)activations.size(); ++i, --k)
    {
        auto activation = activations[i];
        double weight = pow(memory_decay_, k);
        if (const auto &[it, success] = usage_weights.emplace(std::piecewise_construct,
                                                              std::forward_as_tuple(activation.extension_id, activation.item_id),
                                                              std::forward_as_tuple(weight)); !success)
            it->second += weight;
    }

    // Invert the list. Results in ordered by rank map
    map<double, vector<Key>> weight_items;
    for (const auto &[ids, weight] : usage_weights)
        weight_items[weight].emplace_back(ids);

    // Distribute scores linearly over the interval preserving the order
    UsageScores usage_scores;
    double rank = 0.0;
    for (const auto &[weight, vids] : weight_items)
    {
        double score = rank / weight_items.size();
        for (const auto &ids : vids)
            usage_scores.emplace(ids, score);
        rank += 1.0;
    }

    usage_scores_ = ::move(usage_scores);
}


void UsageHistory::db_connect()
{
    DEBG << "Connecting to database…";
    unique_lock lock(db_recursive_mutex_);

    auto db = QSqlDatabase::addDatabase("QSQLITE", db_conn_name);

    // Move db to config location
    auto conf_loc = QDir(configLocation()).absoluteFilePath(db_file_name);
    auto data_loc = QDir(dataLocation()).absoluteFilePath(db_file_name);
    if (QFile::exists(conf_loc)){
        if (QFile::exists(data_loc))
            QFile::moveToTrash(conf_loc);
        else {
            if(!QFile::rename(conf_loc, data_loc))
                CRIT << "Failed to move the usage database to data location";
        }
    }

    if (!db.isValid())
        qFatal("No sqlite available");

    if (!db.driver()->hasFeature(QSqlDriver::Transactions))
        qFatal("QSqlDriver::Transactions not available.");

    db.setDatabaseName(QDir(dataLocation()).filePath(db_file_name));

    if (!db.open())
        qFatal("Database: Unable to establish connection: %s", qPrintable(db.lastError().text()));
}

void UsageHistory::db_initialize()
{
    DEBG << "Initializing database…";
    unique_lock lock(db_recursive_mutex_);

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

void UsageHistory::db_clearActivations()
{
    DEBG << "Clearing activations…";
    unique_lock lock(db_recursive_mutex_);

    QSqlQuery sql(QSqlDatabase::database(db_conn_name));
    sql.exec("DROP TABLE activation;");
    db_initialize();
}

void UsageHistory::db_addActivation(const QString &q, const QString &e, const QString &i, const QString &a)
{
    DEBG << "Database: Adding activation…";
    unique_lock lock(db_recursive_mutex_);

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
