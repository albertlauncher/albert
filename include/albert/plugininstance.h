// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/config.h>
#include <albert/export.h>
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
class ALBERT_EXPORT PluginInstance
{
public:

    /// The widget used to configure the plugin in the settings.
    /// @returns The config widget.
    virtual QWidget *buildConfigWidget();

    /// The extensions provided by this plugin.
    /// @returns Weak references to the extensions.
    virtual std::vector<albert::Extension*> extensions();

public:

    /// The \ref PluginLoader of this instance.
    /// @returns @copybrief loader
    [[nodiscard]] const PluginLoader &loader() const;

    /// The recommended cache location.
    /// @returns @copybrief cacheLocation
    [[nodiscard]] std::filesystem::path cacheLocation() const;

    /// The recommended config location.
    /// @returns @copybrief configLocation
    [[nodiscard]] std::filesystem::path configLocation() const;

    /// The recommended data location.
    /// @returns @copybrief dataLocation
    [[nodiscard]] std::filesystem::path dataLocation() const;

    /// The existing data locations of this plugin.
    /// @returns @copybrief dataLocations
    [[nodiscard]] std::vector<std::filesystem::path> dataLocations() const;

    /// Persistent plugin settings.
    /// Preconfigured according to albert conventions, i.e. using
    /// \ref albert::settings() configured to write to a section titled <plugin-id>.
    /// @returns Preconfigured QSettings object for config storage.
    [[nodiscard]] std::unique_ptr<QSettings> settings() const;

    /// Persistent plugin state.
    /// Preconfigured according to albert conventions, i.e. using
    /// \ref albert::state() configured to write to a section titled <plugin-id>.
    /// @returns Preconfigured QSettings object for state storage.
    [[nodiscard]] std::unique_ptr<QSettings> state() const;

    /// Reads the keychain value for `key`.
    /// Convenience function avoiding name conflicts.
    [[nodiscard]] QString readKeychain(const QString & key) const;

    /// Sets the keychain value of `key` to `value`.
    /// Convenience function avoiding name conflicts.
    void writeKeychain(const QString &key, const QString &value) const;

protected:

    PluginInstance();
    virtual ~PluginInstance();

private:

    class Private;
    std::unique_ptr<Private> d;

};

}

///
/// @brief Declare a class as native Albert plugin.
///
/// Sets the interface identifier to #ALBERT_PLUGIN_IID and uses the metadata file named
/// 'metadata.json' located at CMAKE_CURRENT_SOURCE_DIR.
///
/// This macro has to be put into the plugin class body. The class this macro appears on must be
/// default-constructible, inherit QObject and contain the Q_OBJECT macro. There should be exactly
/// one occurrence of this macro in the source code for a plugin.
///
#define ALBERT_PLUGIN Q_OBJECT Q_PLUGIN_METADATA(IID ALBERT_PLUGIN_IID FILE "metadata.json")
