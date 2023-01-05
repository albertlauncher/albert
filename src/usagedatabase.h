// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QSqlDatabase>
#include <QString>
#include <vector>

struct Activation {
    Activation(QString q, QString e, QString i, QString a);
    QString query;
    QString extension_id;
    QString item_id;
    QString action_id;
};

class UsageDatabase
{
public:
    static void initializeDatabase();
    static void addActivation(const QString &query, const QString &extension, const QString &item, const QString &action);
    static std::vector<Activation> activations();
    static void clearActivations();
};


