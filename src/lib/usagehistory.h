// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <map>
#include <QStringList>

class UsageHistory
{
public:
    static void initializeDatabase();
    static void clearDatabase();
    static void addActivation(const QString& query, const QString &item_id, const QString &action_id);

    static QStringList inputHistory();
    static std::map<QString, double> mruScores();
};


