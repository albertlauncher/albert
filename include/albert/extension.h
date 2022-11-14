// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QDir>
#include <QSettings>
#include <memory>

namespace albert
{
/// Interface for objects of the extension system
struct ALBERT_EXPORT Extension
{
    virtual ~Extension() = default;
    virtual QString id() const = 0;  /// The guid of the extension
    QDir cacheDir() const;  /// The recommended cache location
    QDir configDir() const;  /// The recommended config location
    QDir dataDir() const;  /// The recommended data location
    std::unique_ptr<QSettings> settings() const ;  /// Prepared settings object
};
}

