// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include "export.h"
#include <QDir>
#include <QSettings>
#include <memory>

namespace albert
{
/// Interface for objects of the extension system
class ALBERT_EXPORT Extension
{
public:
    Extension() = default;
    Extension(const Extension&) = delete;
    Extension(Extension&&) = delete;
    Extension& operator=(const Extension&) = delete;
    Extension& operator=(Extension&&) = delete;
    virtual ~Extension() = default;

    virtual QString id() const = 0;  /// The guid of the extension
    virtual QString name() const = 0;  /// Pretty, human readable name
    virtual QString description() const = 0;  /// Brief description of what this extension provides

    QDir cacheDir() const;  /// The recommended cache location
    QDir configDir() const;  /// The recommended config location
    QDir dataDir() const;  /// The recommended data location
    std::unique_ptr<QSettings> settings() const ;  /// Prepared settings object
};
}

