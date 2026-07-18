// Copyright (C) 2026-2026 Manuel Schneider

#pragma once
#include <QStringList>
class QSettings;

class PathManager
{
public:
    PathManager(const QSettings &settings);

    const QStringList &originalPathEntries() const;
    const QStringList &additionalPathEntries() const;

    // Restart reqiured to properly take effect
    void setAdditionalPathEntries(const QStringList &entries);

private:
    QStringList original_path_entries_;
    QStringList additional_path_entries_;
};
