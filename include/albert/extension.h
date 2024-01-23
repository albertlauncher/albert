// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>

namespace albert
{

///
/// The extension interface class.
///
/// This is the interface for classes which want to join the extensions pool.
///
/// \sa ExtensionRegistry
///
class ALBERT_EXPORT Extension
{
public:
    Extension() = default;

    /// The identifier of this extension
    virtual QString id() const = 0;

    /// Pretty, human readable name
    virtual QString name() const = 0;

    /// Brief description of this extension
    virtual QString description() const = 0;

protected:
    virtual ~Extension() = default;
};

}

