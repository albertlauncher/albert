// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>
#include <QSettings>
#include <memory>

namespace albert {
class ALBERT_EXPORT Extension  /// Interface for objects of the extension system
{
public:
    virtual ~Extension() = default;
    virtual QString id() const = 0;  /// The guid of the extension

    QString cacheLocation() const;  /// The recommended cache location
    QString configLocation() const;  /// The recommended config location
    QString dataLocation() const;  /// The recommended data location
    std::unique_ptr<QSettings> settings() const ;  /// Prepared settings object

};
}

