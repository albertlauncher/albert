// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/extension.h>
#include <vector>

namespace albert
{
class PluginLoader;

///
/// Plugin provider interface class.
///
/// \ingroup plugin
///
class ALBERT_EXPORT PluginProvider : virtual public Extension
{
public:

    ///
    /// Returns references to the plugins provided by this plugin provider.
    ///
    /// The calles does **not** take ownership of the returned plugin loaders.
    ///
    virtual std::vector<PluginLoader*> plugins() = 0;

protected:

    ///
    /// Destructs the plugin provider.
    ///
    virtual ~PluginProvider();

};

}
