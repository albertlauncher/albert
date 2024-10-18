// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <filesystem>
class NotificationPrivate;
class QNetworkAccessManager;
class QSettings;
class QUrl;

namespace albert
{
class ExtensionRegistry;

///
/// Restart the application.
///
/// @since 0.27
///
void restart();

///
/// Quit the application.
///
/// @since 0.27
///
void quit();

/// The global QNetworkAccessManager.
/// \return The global QNetworkAccessManager.
ALBERT_EXPORT QNetworkAccessManager *network();

/// Open/Show the settings window (of plugin).
/// \param plugin_id Id of the plugin whose settings should be displayed.
ALBERT_EXPORT void showSettings(QString plugin_id = {});

/// The app config location.
/// The path to the directory where configuration files should be stored.
/// \return The app config location.
ALBERT_EXPORT std::filesystem::path configLocation();

/// The app cache location.
/// The path to the directory where cache files should be stored.
/// \return The app cache location.
ALBERT_EXPORT std::filesystem::path cacheLocation();

/// The app data location.
/// The path to the directory where data files should be stored.
/// \return The app data location.
ALBERT_EXPORT std::filesystem::path dataLocation();

/// Persistent app settings storage.
/// \return Preconfigured QSettings object for configuration storage.
ALBERT_EXPORT std::unique_ptr<QSettings> settings();

/// The persistent app state storage.
/// \return Preconfigured QSettings object for state storage.
ALBERT_EXPORT std::unique_ptr<QSettings> state();

/// Open the albert website in default browser.
ALBERT_EXPORT void openWebsite();

/// Open the specified url in default browser.
/// \param url The url to open
ALBERT_EXPORT void openUrl(const QUrl &url);

/// Open the specified url in default browser.
/// \param url The url to open
ALBERT_EXPORT void openUrl(const QString &url);

///
/// Open URL with default handler.
///
/// \param url The url to open.
/// \since 0.27
///
ALBERT_EXPORT void open(const QUrl &url);

///
/// Open file with default handler.
///
/// \param path The path the file to open.
/// \since 0.27
///
ALBERT_EXPORT void open(const QString &path);

///
/// Open file with default handler.
///
/// Convenience overlaod that works well with filesystem::path.
///
/// \param path The path the file to open.
/// \since 0.27
///
ALBERT_EXPORT void open(const std::string &path);

/// Set the system clipboard.
/// \param text The text to set
ALBERT_EXPORT void setClipboardText(const QString &text);

/// Check paste support of the platform.
/// \return True if requirements for setClipboardTextAndPaste(…) are met.
/// \since 0.24
ALBERT_EXPORT bool havePasteSupport();

/// Set the system clipboard and paste the content to the front-most window.
/// Check albert::havePasteSupport before using this function.
/// \param text The text to set and paste
ALBERT_EXPORT void setClipboardTextAndPaste(const QString &text);

/// Run a detached process.
/// \param commandline The command line to run
/// \param working_dir The working directory
/// \return The process id
ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline, const QString &working_dir = {});

///
/// The extension registry.
///
/// Utilze to look up extensions or watch for changes. Const because registering plugins via this
/// registry is not allowed. Use PluginInstance::extensions().
///
/// See also WeakDependency and StrongDependency.
///
/// @since 0.27
/// @returns A const reference to the extension registry.
///
ALBERT_EXPORT const ExtensionRegistry &extensionRegistry();

///
/// Create a directory if it does not exist yet.
///
/// This is a utility function for use with the *Location functions.
///
/// @param path The path to the directory to create.
/// @returns The existing directory.
/// @throws std::runtime_error if the directory could not be created.
/// @since 0.27
///
ALBERT_EXPORT void tryCreateDirectory(const std::filesystem::path &path);

} // namespace albert

