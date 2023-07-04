// Copyright (c) 2023 Manuel Schneider

#include "albert/extension.h"
#include <QCoreApplication>
#include <QSettings>

std::unique_ptr<QSettings> albert::Extension::settings() const
{
    auto s = std::make_unique<QSettings>(qApp->applicationName());
    s->beginGroup(id());
    return s;
}
