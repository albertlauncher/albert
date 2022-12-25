// Copyright (c) 2021-2022 Manuel Schneider

#include "albert/extension.h"
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QCoreApplication>


static QDir stdDir(QStandardPaths::StandardLocation loc, const QString &id)
{
    QDir dir(QStandardPaths::writableLocation(loc));
    if (!dir.exists(id))
        dir.mkdir(id);
    dir.cd(id);
    return dir;
}

QDir albert::Extension::cacheDir() const
{
    return stdDir(QStandardPaths::CacheLocation, id());
}

QDir albert::Extension::configDir() const
{
    return stdDir(QStandardPaths::AppConfigLocation, id());
}

QDir albert::Extension::dataDir() const
{
    return stdDir(QStandardPaths::AppDataLocation, id());
}

std::unique_ptr<QSettings> albert::Extension::settings() const
{
    auto s = std::make_unique<QSettings>(qApp->applicationName());
    s->beginGroup(id());
    return s;
}
