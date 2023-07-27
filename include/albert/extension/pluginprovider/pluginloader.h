// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QObject>
#include <QString>

namespace albert
{
class ExtensionRegistry;
class PluginProvider;
class PluginInstance;
class PluginMetaData;


/// The loading state of the plugin.
enum class PluginState {
    Invalid,  ///< The plugin does not fulfill the metadata reqiurements.
    Busy,     ///< The plugin is busy loading or unloading.
    Unloaded, ///< The plugin is valid and ready to be loaded.
    Loaded,   ///< The plugin is loaded and ready.
};

/// Abstract plugin loader class
/// @see PluginProvider
class ALBERT_EXPORT PluginLoader : public QObject
{
    Q_OBJECT
public:
    /// PluginLoader constructor
    /// \param path The path to the file of the plugin
    PluginLoader(const QString &path);
    virtual ~PluginLoader();

    const QString path;  ///< The plugin location on disk.
    PluginState state() const; ///< @see PluginState.
    const QString &stateInfo() const;  ///< Detailed state information.

    virtual const PluginProvider &provider() const = 0;  ///< The provider of this plugin.
    virtual const PluginMetaData &metaData() const = 0;  ///< @see PluginMetaData.
    virtual PluginInstance *instance() const = 0;  ///< The plugin instance. nullptr if not loaded.

    /// Load the plugin.
    /// @note To work properly with the plugin registry this has to set state to Busy or Loaded
    virtual QString load(ExtensionRegistry *registry) = 0;

    /// Unload the plugin.
    /// @note To work properly with the plugin registry this has to set state to Busy or Unloaded
    virtual QString unload(ExtensionRegistry *registry) = 0;

protected:
    /// Sets the state of the plugin.
    /// @note It's crucial for the plugin registry and the corresponding
    /// widgets to have the state set correctly to work properly
    void setState(PluginState state, QString info = {});

private:
    QString state_info_;
    PluginState state_;

signals:
    void stateChanged(PluginState);  ///< Emitted when the plugin changed its state
};

}
