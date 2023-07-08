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


class ALBERT_EXPORT PluginLoader : public QObject
{
    Q_OBJECT
public:
    PluginLoader(const QString &path);
    virtual ~PluginLoader();

    const QString path;  ///< The plugin location on disk.
    PluginState state() const; ///< @See PluginState.
    const QString &stateInfo() const;  ///< Detailed state information.

    virtual const PluginProvider &provider() const = 0;  ///< The provider of this plugin.
    virtual const PluginMetaData &metaData() const = 0;  ///< @See PluginMetaData.
    virtual PluginInstance *instance() const = 0;  ///< The plugin instance. nullptr if not loaded.

    /// Load the plugin.
    /// @note To work properly with the plugin registry this has to set state to Busy or Loaded
    virtual QString load(ExtensionRegistry *registry) = 0;

    /// Unload the plugin.
    /// @note To work properly with the plugin registry this has to set state to Busy or Unloaded
    virtual QString unload(ExtensionRegistry *registry) = 0;

protected:
    void setState(PluginState state, QString info = {});

private:
    QString state_info_;
    PluginState state_;

signals:
    void stateChanged(PluginState);
};

}
