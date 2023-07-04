// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>
class QSettings;

namespace albert
{

///< Interface for objects of the extension system
class ALBERT_EXPORT Extension
{
public:
    virtual ~Extension() = default;
    virtual QString id() const = 0;  ///< The guid of the extension
    virtual QString name() const = 0;  ///< Pretty, human readable name
    virtual QString description() const = 0;  ///< Brief description of what this extension provides

    std::unique_ptr<QSettings> settings() const;  ///< Prepared settings object.
};


}

