// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

/// \defgroup plugin Plugin API
/// Classes and functions related to the plugin system.

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
/// \ingroup plugin
///
class ALBERT_EXPORT PluginInstance : public QObject
{
public:

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
    virtual std::vector<albert::Extension*> extensions();

public:

    ///
    /// Returns the loader of this plugin.
    ///
    [[nodiscard]] const PluginLoader &loader() const;

    ///
    /// Returns the writable cache location for this plugin.
    ///
    [[nodiscard]] std::filesystem::path cacheLocation() const;

    ///
    /// Returns the writable config location for this plugin.
    ///
    [[nodiscard]] std::filesystem::path configLocation() const;

    ///
    /// Returns the writable data location for this plugin.
    ///
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

    ///
    /// Reads the keychain value for _key_ asynchronously.
    ///
    /// Calls _onSuccess_ with the _value_ of the _key_ on success and `onError` with an _error_ message on failure.
    ///
    void readKeychain(const QString &key,
                      std::function<void(const QString &value)> onSuccess,
                      std::function<void(const QString &error)> onError) const;

    ///
    /// Sets the keychain value of _key_ to _value_ asynchronously.
    ///
    /// Calls _onSuccess_ on success and _onError_ with an _error_ message on failure.
    ///
    void writeKeychain(const QString &key,
                       const QString &value,
                       std::function<void()> onSuccess,
                       std::function<void(const QString&error)> onError) const;

protected:

    ///
    /// Constructs a plugin instance.
    ///
    PluginInstance();

    ///
    /// Destructs the plugin instance.
    ///
    virtual ~PluginInstance();

private:

    class Private;
    std::unique_ptr<Private> d;

};

}
