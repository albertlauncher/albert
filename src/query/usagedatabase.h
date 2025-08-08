// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include "usagescoring.h"  // ItemKey
#include <QString>
#include <map>
class QDateTime;

//
// Direct database access.
// Use in main thread only!
//
class UsageDatabase
{
public:

    static UsageDatabase &instance();

    std::map<QString, uint> extensionActivationsSince(const QDateTime &query) const;

    std::unordered_map<ItemKey, double> itemUsageScores(double memory_decay) const;

    void addActivation(const QString &query,
                       const QString &extension,
                       const QString &item,
                       const QString &action) const;

    void clearActivations() const;

private:

    UsageDatabase();

};
