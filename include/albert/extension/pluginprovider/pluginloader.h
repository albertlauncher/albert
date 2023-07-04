// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>

namespace albert
{
class PluginProvider;
class PluginInstance;
class PluginMetaData;

class ALBERT_EXPORT PluginLoader
{
public:

    /// The loading state of the plugin.
    enum class PluginState {
        Invalid,  ///< Plugin does not fulfill the reqiurements.
        Unloaded,  ///< The plugin is valid and ready to be loaded.
        Initializing,  /// Pluin is currently beeing initialized.
        Loaded  ///< The plugin is loaded.
    };

    PluginLoader(const QString &path);
    virtual ~PluginLoader();

    const QString path;  ///< The plugin location on disk.
    PluginState state() const; ///< @See PluginState.
    const QString &stateInfo() const;  ///< Detailed state information.

    virtual PluginProvider *provider() const = 0;  ///< The provider of this plugin.
    virtual const PluginMetaData &metaData() const = 0; ///< @See PluginMetaData.
    virtual PluginInstance *instance() const = 0;  ///< The plugin instance. nullptr if not loaded.

    virtual void load() = 0;  ///< Loads the plugin
    virtual void unload() = 0;   ///< Unloads the plugin

protected:
    PluginState state_;
    QString state_info_;
};

}
