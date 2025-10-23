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
class NotificationPrivate;
class QNetworkAccessManager;
class QSettings;
class QUrl;

namespace albert
{
class ExtensionRegistry;
class UsageScoring;

/// @name App functions
/// @addtogroup core
/// @{

///
/// Shows the frontend and optionally sets the text to _input_text_.
///
ALBERT_EXPORT void show(const QString &input_text = {});

///
/// Shows the settings window and optionally selects the plugin with _plugin_id_.
///
ALBERT_EXPORT void showSettings(QString plugin_id = {});

///
/// Restarts the application.
///
/// This function is thread-safe.
///
ALBERT_EXPORT void restart();

///
/// Quits the application.
///
/// This function is thread-safe.
///
ALBERT_EXPORT void quit();

///
/// Returns the application config location.
///
/// This function is thread-safe.
///
ALBERT_EXPORT const std::filesystem::path &configLocation();

///
/// Returns the application cache location.
///
/// This function is thread-safe.
///
ALBERT_EXPORT const std::filesystem::path &cacheLocation();

///
/// Returns the application data location.
///
/// This function is thread-safe.
///
ALBERT_EXPORT const std::filesystem::path &dataLocation();

///
/// Returns a QSettings object for configuration storage.
///
/// This function is thread-safe.
///
ALBERT_EXPORT std::unique_ptr<QSettings> settings();

///
/// Returns a QSettings object for state storage.
///
/// This function is thread-safe.
///
ALBERT_EXPORT std::unique_ptr<QSettings> state();

///
/// Returns a const reference to the central \ref ExtensionRegistry.
///
/// Registering plugins via this registry is not allowed. Use \ref PluginInstance::extensions().
/// See also \ref WeakDependency and \ref StrongDependency.
///
ALBERT_EXPORT const ExtensionRegistry &extensionRegistry();

///
/// Returns the global usage scoring.
///
/// This function is thread-safe.
///
ALBERT_EXPORT UsageScoring usageScoring();

/// @}

} // namespace albert
