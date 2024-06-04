// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>

namespace albert
{
class PluginInstance;
class PluginMetaData;

///
/// Plugin loader interface class.
///
class ALBERT_EXPORT PluginLoader
{
public:

    /// The path to the plugin.
    /// @return @copybrief path
    virtual QString path() const = 0;

    /// The plugin metadata.
    /// @return @copybrief metaData
    virtual const PluginMetaData &metaData() const = 0;

    /// Load the plugin.
    /// Called in a background thread.
    /// Expects the plugin to be loaded after this call.
    /// @throws std::exception in case of errors.
    virtual void load() = 0;

    /// Unload the plugin.
    /// @throws std::exception in case of errors.
    virtual void unload() = 0;

    /// The plugin instance.
    /// Not called unless loaded.
    /// Creates an instance of the plugin if it does not exist.
    /// @return @copybrief createInstance
    virtual PluginInstance *createInstance() = 0;

protected:

    virtual ~PluginLoader();

};

}
