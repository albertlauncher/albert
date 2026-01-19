// Copyright (C) 2022-2025 Manuel Schneider

#pragma once
#include <memory>
#include <QStringList>
class QSettings;

class PathManager
{
public:

    PathManager(const QSettings &settings);
    ~PathManager();

    const QStringList &originalPathEntries() const;
    const QStringList &additionalPathEntries() const;
    void setAdditionalPathEntries(const QStringList &entries);

private:
    class Private;
    std::unique_ptr<Private> d;

};
