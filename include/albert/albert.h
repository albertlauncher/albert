// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <filesystem>
#include <albert/export.h>
class NotificationPrivate;
class QNetworkAccessManager;
class QSettings;
class QUrl;

namespace albert
{
class ExtensionRegistry;

/// Shows the frontend.
/// If _input_text_ is not null the input is set.
ALBERT_EXPORT void show(const QString &input_text = {});

/// Creates and/or shows the settings window.
/// If specified the settings of the plugin with the id _plugin_id_ are shown.
ALBERT_EXPORT void showSettings(QString plugin_id = {});

/// Restarts the application.
/// This function is thread-safe.
ALBERT_EXPORT void restart();

/// Quits the application.
/// This function is thread-safe.
ALBERT_EXPORT void quit();

/// Returns the application config location.
/// This function is thread-safe.
ALBERT_EXPORT const std::filesystem::path &configLocation();

/// Returns the application cache location.
/// This function is thread-safe.
ALBERT_EXPORT const std::filesystem::path &cacheLocation();

/// Returns the application data location.
/// This function is thread-safe.
ALBERT_EXPORT const std::filesystem::path &dataLocation();

/// Returns a QSettings object for configuration storage.
/// This function is thread-safe.
ALBERT_EXPORT std::unique_ptr<QSettings> settings();

/// Returns a QSettings object for state storage.
/// This function is thread-safe.
ALBERT_EXPORT std::unique_ptr<QSettings> state();

/// Returns a const reference to the central \ref ExtensionRegistry.
/// Registering plugins via this registry is not allowed. Use \ref PluginInstance::extensions().
/// See also \ref WeakDependency and \ref StrongDependency.
ALBERT_EXPORT const ExtensionRegistry &extensionRegistry();

} // namespace albert
