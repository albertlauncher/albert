// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <QString>
#include <albert/export.h>

namespace albert
{
class PluginInstance;
class PluginMetadata;

///
/// Asynchronous plugin loader turning a physical plugin into a logical \ref PluginInstance.
///
/// \ingroup plugin
///
class ALBERT_EXPORT PluginLoader : public QObject
{
    Q_OBJECT

public:

    ///
    /// Returns the path to the plugin.
    ///
    virtual QString path() const = 0;

    ///
    /// Returns the plugin metadata.
    ///
    virtual const PluginMetadata &metadata() const = 0;

    ///
    /// Starts loading the plugin.
    ///
    /// @sa \ref current_loader
    ///
    virtual void load() = 0;

    ///
    /// Unloads the plugin.
    ///
    virtual void unload() = 0;

    ///
    /// Returns the \ref PluginInstance if the plugin is loaded, else `nullptr`.
    ///
    virtual albert::PluginInstance *instance() = 0;

    ///
    /// Used to set the plugin loader while plugin instatiation.
    ///
    /// This avoids injection mechanisms and therefore reduces boilerplate for \ref PluginInstance
    /// implementations. Implementations have to set this before calling the constructor of the
    /// plugin instance.
    ///
    static thread_local PluginLoader *current_loader;

signals:

    ///
    /// Emitted when the loading process finished.
    ///
    /// On success \ref instance() returns a valid \ref PluginInstance. _info_ contains additional
    /// information about the finished loading proc \ref instance() returns a nullptr, i.e. the
    /// plugin failed to load, _info_ is interpreted as error message.
    ///
    void finished(QString info);

protected:

    virtual ~PluginLoader();

};

}
