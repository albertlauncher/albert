// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <QString>
#include <albert/export.h>
#include <albert/plugin.h>
#include <filesystem>
#include <memory>
#include <vector>
class QSettings;
class QWidget;

namespace albert
{
class Extension;
class PluginLoader;

///
/// Abstract plugin instance class.
///
/// The class every plugin has to inherit.
///
/// If the plugin instantiation fails you are supposed to print errors in english to the logs and
/// throw a localized message that will be shown to the user.
///
/// \ingroup core_plugin
///
class ALBERT_EXPORT PluginInstance : public QObject
{
    Q_OBJECT

public:

    ///
    /// Triggers the asynchronous initialization.
    ///
    /// Implementations have to emit \ref initialized() or call base::initialize() when done
    /// initializing.
    ///
    virtual void initialize();

    ///
    /// Creates a widget that can be used to configure the plugin properties.
    ///
    /// The caller takes ownership of the returned object.
    ///
    virtual QWidget *buildConfigWidget();

    ///
    /// Returns the extensions provided by this plugin.
    ///
    /// The caller does **not** take ownership of the returned objects.
    ///
    virtual std::vector<albert::Extension *> extensions();

public:
    /// Returns the loader of this plugin.
    [[nodiscard]] const PluginLoader &loader() const;

    /// Returns the writable cache location for this plugin.
    [[nodiscard]] std::filesystem::path cacheLocation() const;

    /// Returns the writable config location for this plugin.
    [[nodiscard]] std::filesystem::path configLocation() const;

    /// Returns the writable data location for this plugin.
    [[nodiscard]] std::filesystem::path dataLocation() const;

    ///
    /// Returns the existing data locations for this plugin.
    ///
    /// This includes user, vendor, and system locations.
    ///
    [[nodiscard]] std::vector<std::filesystem::path> dataLocations() const;

    ///
    /// Creates a preconfigured `QSettings` object for plugin config data.
    ///
    /// Configured to use the group <plugin-id> in \ref albert::config().
    ///
    [[nodiscard]] std::unique_ptr<QSettings> settings() const;

    ///
    /// Creates a preconfigured `QSettings` object for plugin state data.
    ///
    /// Configured to use the group <plugin-id> in \ref albert::state().
    ///
    [[nodiscard]] std::unique_ptr<QSettings> state() const;

signals:

    /// Emitted when the plugin has completed initialization.
    void initialized();

protected:
    /// Constructs a plugin instance.
    PluginInstance();

    /// Destructs the plugin instance.
    virtual ~PluginInstance();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace albert
