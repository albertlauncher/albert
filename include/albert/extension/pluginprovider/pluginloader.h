// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QObject>
#include <QString>
#include <memory>
class PluginRegistry;

namespace albert
{
class ExtensionRegistry;
class PluginProvider;
class PluginInstance;
class PluginMetaData;


/// The state of the plugin.
enum class PluginState {
    Unloaded, ///< The plugin is valid and ready to be loaded.
    Busy,     ///< The plugin is busy loading or unloading.
    Loaded,   ///< The plugin is loaded and ready.
};

/// Abstract plugin loader class used by the plugin registry.
/// Instanciated by a PluginProvider.
///
class ALBERT_EXPORT PluginLoader : public QObject
{
    Q_OBJECT
public:
    /// PluginLoader constructor.
    /// \param path \copydoc path
    PluginLoader(const QString &path);
    virtual ~PluginLoader();

    /// The plugin location on disk.
    const QString path;

    /// The provider of this plugin.
    /// @returns the PluginProvider of the plugin.
    virtual const PluginProvider &provider() const = 0;

    /// The plugin metadata.
    /// @returns The PluginMetaData of the plugin.
    virtual const PluginMetaData &metaData() const = 0;

    /// @copybrief PluginState
    /// @returns The PluginState of the plugin.
    PluginState state() const;

    /// Detailed information about the latest state change.
    /// @returns A string containing detailed information about the latest
    /// state change.
    const QString &stateInfo() const;

    /// Returns the plugin instance.
    /// @returns The PluginInstance or nullptr if not loaded.
    virtual PluginInstance *instance() const = 0;

protected:
    /// Loads the plugin.
    /// On success instance() should return a valid object.
    /// @return Errorstring, if any
    virtual QString load() = 0;

    /// Unloads the plugin.
    /// @return Errorstring, if any
    virtual QString unload() = 0;

    class PluginLoaderPrivate;
    const std::unique_ptr<PluginLoaderPrivate> d;
    friend class ::PluginRegistry;

signals:
    void stateChanged();
};

}
