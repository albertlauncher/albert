// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
class QString;

namespace albert
{

///
/// Abstract extension class.
///
/// Inherited by classes that want to join the extensions pool of the \ref ExtensionRegistry.
///
/// \ingroup core_extension
///
class ALBERT_EXPORT Extension
{
public:
    ///
    /// Returns the extension identifier.
    ///
    /// To avoid naming conflicts use the namespace of your plugin,
    /// e.g. files (root extension), files.rootbrowser, files.homebrowser, …
    ///
    virtual QString id() const = 0;

    ///
    /// Returns the pretty, human readable extension name.
    ///
    virtual QString name() const = 0;

    ///
    /// Returns the brief extension description.
    ///
    virtual QString description() const = 0;

protected:
    ///
    /// Destructs the extension.
    ///
    virtual ~Extension();
};

}  // namespace albert
