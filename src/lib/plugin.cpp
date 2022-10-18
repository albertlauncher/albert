// Copyright (C) 2014-2018 Manuel Schneider

#include <QStandardPaths>
#include <QDir>
#include "plugin.h"


extern albert::PluginSpec *current_spec_in_construction;

albert::Plugin::Plugin() : spec(*current_spec_in_construction){}

QString albert::Plugin::id() const
{
    return spec.id;
}

QString albert::Plugin::cacheLocation() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).filePath(spec.id);
}

QString albert::Plugin::configLocation() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).filePath(spec.id);
}

QString albert::Plugin::dataLocation() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(spec.id);
}
