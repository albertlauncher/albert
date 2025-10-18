// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QtGlobal>
#include <albert/export.h>
#include <filesystem>
class QString;
class QUrl;
template <typename T> class QList;
typedef QList<QString> QStringList;

namespace albert
{

/// @name System utility
/// @addtogroup core
/// @{

///
/// Opens _url_ with the default handler for the scheme.
///
/// Does nothing if _url_ is not a valid URL.
///
ALBERT_EXPORT void openUrl(const QString &url);

///
/// Opens _url_ with the default handler for the scheme.
///
ALBERT_EXPORT void open(const QUrl &url);

///
/// Opens a file at _path_ with the associated default application.
///
ALBERT_EXPORT void open(const QString &path);

///
/// Opens a file at _path_ with the associated default application.
///
ALBERT_EXPORT void open(const std::filesystem::path &path);

///
/// Sets the system clipboard to _text_.
///
ALBERT_EXPORT void setClipboardText(const QString &text);

///
/// Returns the `true` if the platform supports pasting, else `false`.
///
ALBERT_EXPORT bool havePasteSupport();

///
/// Sets the system clipboard to _text_ and pastes _text_ to the front-most window.
///
/// Check \ref albert::havePasteSupport before using this function.
///
ALBERT_EXPORT void setClipboardTextAndPaste(const QString &text);

///
/// Starts the _commandline_ in a new process, and detaches from it.
///
/// Returns the PID on success; otherwise returns 0.
/// The working directory is the users home directory.
///
ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline);

///
/// Starts the _commandline_ in a new process, and detaches from it.
///
/// Returns the PID on success; otherwise returns 0.
/// The process will be started in the directory `working_dir`.
/// If `working_dir` is empty, the working directory is the users home directory.
///
ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline, const QString &working_dir);

/// Returns a QString representation of _path_.
///
ALBERT_EXPORT QString toQString(const std::filesystem::path &path);

#ifdef Q_OS_MAC
///
/// Execute the AppleScript _script_. Returns any return value of the script.
///
/// Throws runtime_error in case of an error.
/// Available on macOS only.
///
ALBERT_EXPORT QString runAppleScript(const QString &script);
#endif

/// @}

}
