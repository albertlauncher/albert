// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>

namespace albert
{
class PluginInstance;
class PluginMetaData;

///
/// Plugin loader interface class.
///
/// @since 0.23
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
    /// @note Will be executed in a background thread.
    virtual void load() = 0;

    /// Unload the plugin.
    /// @note Will be executed in a background thread.
    virtual void unload() = 0;

    /// The plugin instance.
    /// @return The PluginInstance or nullptr if not loaded.
    virtual PluginInstance *createInstance() = 0;

protected:

    virtual ~PluginLoader();

};

}
