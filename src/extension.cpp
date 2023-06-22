// Copyright (c) 2023 Manuel Schneider

#include "albert/extension.h"
#include "albert/albert.h"
#include <QDir>
#include <QSettings>
#include <QCoreApplication>


static QDir make_extension_dir(const QString &location, const QString &id)
{
    QDir dir(location);
    if (!dir.exists(id))
        if (!dir.mkdir(id))
            qFatal("Failed to create writable dir at: %s", qPrintable(dir.filePath(id)));

    dir.cd(id);
    return dir;
}

QDir albert::Extension::cacheDir() const
{ return make_extension_dir(albert::cacheLocation(), id()); }

QDir albert::Extension::configDir() const
{ return make_extension_dir(albert::configLocation(), id()); }

QDir albert::Extension::dataDir() const
{ return make_extension_dir(albert::dataLocation(), id()); }

std::unique_ptr<QSettings> albert::Extension::settings() const
{
    auto s = std::make_unique<QSettings>(qApp->applicationName());
    s->beginGroup(id());
    return s;
}
