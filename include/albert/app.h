// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

/// \defgroup core Core API
/// Core classes and functions.

/// \defgroup util Utility API
/// Utility classes and helper functions.

#pragma once
#include <QString>
#include <albert/export.h>
#include <filesystem>
#include <memory>
class QNetworkAccessManager;
class QSettings;
class QUrl;

namespace albert
{
class ExtensionRegistry;
class UsageScoring;

///
/// The public app instance interface.
///
/// \ingroup core
///
class ALBERT_EXPORT App
{
public:
    ///
    /// Shows the frontend and optionally sets the text to _input_text_.
    ///
    virtual void show(const QString &input_text = {}) = 0;

    ///
    /// Shows the settings window and optionally selects the plugin with _plugin_id_.
    ///
    virtual void showSettings(QString plugin_id = {}) = 0;

    ///
    /// Returns a const reference to the central extension registry.
    ///
    /// Registering plugins via this registry is not allowed. Use \ref PluginInstance::extensions().
    /// See also \ref WeakDependency and \ref StrongDependency.
    ///
    virtual const ExtensionRegistry &extensionRegistry() const = 0;

    ///
    /// Restarts the application.
    ///
    /// This function is thread-safe.
    ///
    static void restart();

    ///
    /// Quits the application.
    ///
    /// This function is thread-safe.
    ///
    static void quit();

    ///
    /// Returns the path to the application config directory.
    ///
    /// This function is thread-safe.
    ///
    static const std::filesystem::path &configLocation();

    ///
    /// Returns the path to the application cache directory.
    ///
    /// This function is thread-safe.
    ///
    static const std::filesystem::path &cacheLocation();

    ///
    /// Returns the path to the application data directory.
    ///
    /// This function is thread-safe.
    ///
    static const std::filesystem::path &dataLocation();

    ///
    /// Returns a `QSettings` object initialized with the application configuration file path.
    ///
    /// As `unique_ptr` for the sake of movability.
    ///
    /// This function is thread-safe.
    ///
    static std::unique_ptr<QSettings> settings();

    ///
    /// Returns a `QSettings` object initialized with the application state file path.
    ///
    /// As `unique_ptr` for the sake of movability.
    ///
    /// This function is thread-safe.
    ///
    static std::unique_ptr<QSettings> state();

    ///
    /// Returns the core app instance.
    ///
    static App &instance();

protected:

    App();
    virtual ~App();

};

} // namespace albert
