// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QDir>
#include <QString>
#include <albert/config.h>
#include <albert/export.h>
#include <filesystem>
#include <memory>
class QSettings;
class QWidget;

namespace albert
{

class Extension;
class ExtensionRegistry;
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
    /// @since 0.27
    virtual std::vector<albert::Extension*> extensions();


public:

    /// The PluginLoader of this instance.
    /// @returns @copybrief loader
    /// @since 0.24
    [[nodiscard]] const PluginLoader &loader() const;

    /// The associated ExtensionRegistry.
    /// @returns @copybrief registry
    /// @since 0.24
    albert::ExtensionRegistry &registry();

    /// The recommended cache location.
    /// @returns @copybrief cacheLocation
    /// @since 0.27
    [[nodiscard]] std::filesystem::path cacheLocation() const;

    /// The recommended config location.
    /// @returns @copybrief configLocation
    /// @since 0.27
    [[nodiscard]] std::filesystem::path configLocation() const;

    /// The recommended data location.
    /// @returns @copybrief dataLocation
    /// @since 0.27
    [[nodiscard]] std::filesystem::path dataLocation() const;

    /// Creates a directory, throws an exception if it fails.
    /// This is a utility function for use with the *Location functions.
    /// @returns The directory.
    /// @since 0.24
    static QDir createOrThrow(const QString &path);

    /// Persistent plugin settings.
    /// Preconfigured according to albert conventions, i.e. using
    /// albert::settings() configured to write to a section titled <plugin-id>.
    /// @returns Preconfigured QSettings object for config storage.
    [[nodiscard]] std::unique_ptr<QSettings> settings() const;

    /// Persistent plugin state.
    /// Preconfigured according to albert conventions, i.e. using
    /// albert::state() configured to write to a section titled <plugin-id>.
    /// @since 0.23
    /// @returns Preconfigured QSettings object for state storage.
    [[nodiscard]] std::unique_ptr<QSettings> state() const;

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
/// Sets the interface identifier to #ALBERT_PLUGIN_IID and uses the metadata
/// file named 'metadata.json' located at CMAKE_CURRENT_SOURCE_DIR.
///
/// @note This macro has to be put into the plugin class body.
/// The class this macro appears on must be default-constructible, inherit
/// QObject and contain the Q_OBJECT macro. There should be exactly one
/// occurrence of this macro in the source code for a plugin.
///
#define ALBERT_PLUGIN Q_OBJECT Q_PLUGIN_METADATA(IID ALBERT_PLUGIN_IID FILE "metadata.json")
