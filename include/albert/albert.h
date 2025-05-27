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

/// Shows the frontend. If `input_text` is not null the input is set.
ALBERT_EXPORT void show(const QString &input_text = {});

/// Restarts the application.
ALBERT_EXPORT void restart();

/// Quits the application.
ALBERT_EXPORT void quit();

/// Opens/Shows the settings window (of `plugin_id`).
ALBERT_EXPORT void showSettings(QString plugin_id = {});

/// Returns the application config location.
ALBERT_EXPORT std::filesystem::path configLocation();

/// Returns the application cache location.
ALBERT_EXPORT std::filesystem::path cacheLocation();

/// Returns the application data location.
ALBERT_EXPORT std::filesystem::path dataLocation();

/// Returns a QSettings object for configuration storage.
ALBERT_EXPORT std::unique_ptr<QSettings> settings();

/// Returns a QSettings object for state storage.
ALBERT_EXPORT std::unique_ptr<QSettings> state();

/// Returns a const reference to the extension registry.
/// Registering plugins via this registry is not allowed. Use PluginInstance::extensions().
/// See also WeakDependency and StrongDependency.
ALBERT_EXPORT const ExtensionRegistry &extensionRegistry();

} // namespace albert

