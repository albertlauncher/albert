// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
class NotificationPrivate;
class QNetworkAccessManager;
class QSettings;
class QUrl;

namespace albert
{

/// The global QNetworkAccessManager.
/// \return The global QNetworkAccessManager.
ALBERT_EXPORT QNetworkAccessManager *network();

/// Open/Show the settings window (of plugin).
/// \param plugin_id Id of the plugin whose settings should be displayed.
ALBERT_EXPORT void showSettings(QString plugin_id = {});

/// The app config location.
/// The path to the directory where configuration files should be stored.
/// \return The app config location.
ALBERT_EXPORT QString configLocation();

/// The app cache location.
/// The path to the directory where cache files should be stored.
/// \return The app cache location.
ALBERT_EXPORT QString cacheLocation();

/// The app data location.
/// The path to the directory where data files should be stored.
/// \return The app data location.
ALBERT_EXPORT QString dataLocation();

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

/// Run a script in the user shell and user defined terminal.
/// \param script The script to run
/// \param working_dir The working directory
/// \param close_on_exit Close the terminal on exit
ALBERT_EXPORT void runTerminal(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false);

}
