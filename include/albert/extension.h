// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>

namespace albert
{

///
/// Abstract extension class.
///
/// Inherited by classes that want to join the extensions pool.
///
/// \sa ExtensionRegistry
///
class ALBERT_EXPORT Extension
{
public:

    /// The identifier of this extension.
    /// @note To avoid naming conflicts use the namespace of your plugin,
    /// e.g. files (root extension), files.rootbrowser, files.homebrowser, …
    virtual QString id() const = 0;

    /// Pretty, human readable name
    virtual QString name() const = 0;

    /// Brief description of this extension
    virtual QString description() const = 0;

protected:

    virtual ~Extension();

};

}

