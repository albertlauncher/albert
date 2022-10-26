// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <map>
#include <QStringList>

class UsageHistory
{
public:
    static QStringList inputHistory();
    static std::map<QString, double> mruScores();

    static void initializeDatabase();
    static void clearDatabase();
    static void updateCache();

    static void addActivation(const QString& query, const QString &item_id, const QString &action_id);
};


