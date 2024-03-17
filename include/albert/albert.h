// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>
class QNetworkAccessManager;
class QSettings;
class QUrl;
class NotificationPrivate;

namespace albert
{

/// The global QNetworkAccessManager.
ALBERT_EXPORT QNetworkAccessManager *networkManager();

/// Show the main window.
ALBERT_EXPORT void show(const QString &text = QString());

/// Hide the main window
ALBERT_EXPORT void hide();

/// Toggle visibility of main window.
ALBERT_EXPORT void toggle();

/// Open/Show the settings window (of plugin).
/// @param plugin_id Id of the plugin whose settings should be displayed.
ALBERT_EXPORT void showSettings(QString plugin_id = {});

/// Restart the app.
ALBERT_EXPORT void restart();

/// Quit the app.
ALBERT_EXPORT void quit();

/// The app config location.
ALBERT_EXPORT QString configLocation();

/// The app cache location.
ALBERT_EXPORT QString cacheLocation();

/// The app data location.
ALBERT_EXPORT QString dataLocation();

/// Persistent app settings. @reentrant
ALBERT_EXPORT std::unique_ptr<QSettings> settings();

/// The persistent app state. @reentrant
ALBERT_EXPORT std::unique_ptr<QSettings> state();

/// Open the albert website in default browser.
ALBERT_EXPORT void openWebsite();

/// Open the specified url in default browser.
ALBERT_EXPORT void openUrl(const QUrl &url);

/// Open the specified url in default browser.
ALBERT_EXPORT void openUrl(const QString &url);

/// Set the system clipboard.
ALBERT_EXPORT void setClipboardText(const QString &text);

/// Returns true if requirements for setClipboardTextAndPaste(…) are met.
ALBERT_EXPORT bool havePasteSupport();

/// Set the system clipboard and paste the content to the front-most window.
/// Check havePasteSupport before using this function.
ALBERT_EXPORT void setClipboardTextAndPaste(const QString &text);

/// Run a detached process.
ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline, const QString &working_dir = {});

/// Run a script in the user defined terminal.
ALBERT_EXPORT void runTerminal(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false);

/// A system tray notification.
/// The notification is visible for as long as this object exists.
class ALBERT_EXPORT Notification
{
public:
    Notification(const QString &title={}, const QString &body={});
    ~Notification();

private:
    NotificationPrivate *d;
};}
