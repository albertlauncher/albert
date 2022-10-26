// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "extension.h"
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QCoreApplication>


QString albert::Extension::cacheLocation() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).filePath(id());
}

QString albert::Extension::configLocation() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).filePath(id());
}

QString albert::Extension::dataLocation() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(id());
}

std::unique_ptr<QSettings> albert::Extension::settings() const
{
    auto s = std::make_unique<QSettings>(qApp->applicationName());
    s->beginGroup(id());
    return s;
}
