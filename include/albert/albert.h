// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "export.h"
#include <QString>
class QNetworkAccessManager;
class QSettings;
class QUrl;

namespace albert
{
class Frontend;

/// The global QNetworkAccessManager
ALBERT_EXPORT QNetworkAccessManager *networkManager();

/// Show the main window
ALBERT_EXPORT void show(const QString &text = QString());

/// Hide the main window
/// @note This applies some platform dependent logic considering the
/// applications activation state (macos) depening on the visibility of the
/// settings window. Notably the settingswindow, which is not intended to stay
/// open, is brought to front if the frontend is hidden.
ALBERT_EXPORT void hide();

/// Toggle visibility of main window
ALBERT_EXPORT void toggle();

/// Open/Show the settings window
ALBERT_EXPORT void showSettings();

/// Restart the app
ALBERT_EXPORT void restart();

/// Quit the app
ALBERT_EXPORT void quit();

/// The app config location
ALBERT_EXPORT QString configLocation();

/// The app cache location
ALBERT_EXPORT QString cacheLocation();

/// The app data location
ALBERT_EXPORT QString dataLocation();

/// Persistent app settings. @note Returns new QSettings object, therefore as stated in the docs reentrant.
ALBERT_EXPORT std::unique_ptr<QSettings> settings();

/// The persistent app state @note Returns new QSettings object, therefore as stated in the docs reentrant.
ALBERT_EXPORT std::unique_ptr<QSettings> state();

/// The frontend of the app
ALBERT_EXPORT Frontend *frontend();

/// Open the albert website in default browser
ALBERT_EXPORT void openWebsite();

/// Open the specified url in default browser
ALBERT_EXPORT void openUrl(const QUrl &url);

/// Open the specified url in default browser
ALBERT_EXPORT void openUrl(const QString &url);

/// Open the albert issue tracker in default browser
ALBERT_EXPORT void openIssueTracker();

/// Set the system clipboard
ALBERT_EXPORT void setClipboardText(const QString &text);

/// Run a detaches process
ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline, const QString &working_dir = {});

/// Run a script in the user defined terminal
ALBERT_EXPORT void runTerminal(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false);

/// Send a tray notification
/// @note Wont display if system tray is disabled.
/// @note Unfortunately broken on most systems
ALBERT_EXPORT void sendTrayNotification(const QString &title, const QString &message, int msTimeoutHint);

}
