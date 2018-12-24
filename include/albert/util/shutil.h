// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QString>
#include <QStringList>
#include "../core_globals.h"

namespace Core {
namespace ShUtil {

EXPORT_CORE QString quote(QString input);
EXPORT_CORE QStringList split(const QString &input);

}
}

