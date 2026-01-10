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
/// Asynchronous plugin loader.
///
/// Turns a physical plugin into a logical plugin instance.
///
/// Errors are intentionally not logged. Thats the responsibility of the plugin implementation. On
/// errors, implementations are expected to throw a localized message and print english logs using
/// their logging category.
///
/// Implementations have to set \ref current_loader before calling the constructor of the plugin
/// instance. This avoids injection mechanisms and therefore reduces boilerplate for \ref
/// PluginInstance implementations.
///
/// Implementations have to emit \ref finished(), when the loading process finished. On success \ref
/// instance() returns a valid \ref PluginInstance. _info_ contains additional information. On error
/// \ref instance() returns a nullptr, i.e. the plugin failed to load, _info_ is an error message.
///
/// \ingroup core_plugin
///
class ALBERT_EXPORT PluginLoader : public QObject
{
    Q_OBJECT

public:
    /// Returns the path to the plugin.
    virtual QString path() const = 0;

    /// Returns the plugin metadata.
    virtual const PluginMetadata &metadata() const = 0;

    /// Starts asynchronous loading process of the plugin.
    virtual void load() = 0;

    /// Unloads the plugin.
    virtual void unload() = 0;

    /// Returns the \ref PluginInstance if the plugin is loaded, else `nullptr`.
    virtual albert::PluginInstance *instance() = 0;

    /// The static injection pointer.
    static thread_local PluginLoader *current_loader;

signals:

    /// Emitted when the loading process finished.
    void finished(QString info);

protected:

    virtual ~PluginLoader();

};

}
