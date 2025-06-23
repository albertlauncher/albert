// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/export.h>
#include <filesystem>
class QString;
class QUrl;
template <typename T> class QList;
typedef QList<QString> QStringList;

namespace albert::util
{

/// Opens `url` with the default handler for the scheme.
/// Does nothing if `url` is not a valid URL.
ALBERT_EXPORT void openUrl(const QString &url);

/// Opens `url` with the default handler for the scheme.
ALBERT_EXPORT void open(const QUrl &url);

/// Opens a file at `path` with the associated default application.
ALBERT_EXPORT void open(const QString &path);

/// Opens a file at `path` with the associated default application.
ALBERT_EXPORT void open(const std::filesystem::path &path);

/// Sets the system clipboard to `text`.
ALBERT_EXPORT void setClipboardText(const QString &text);

/// Returns the `true` if the platform supports pasting, else `false`.
ALBERT_EXPORT bool havePasteSupport();

/// Sets the system clipboard to `text` and pastes `text` to the front-most window.
/// Check albert::havePasteSupport before using this function.
ALBERT_EXPORT void setClipboardTextAndPaste(const QString &text);

/// Run the `commandline` as detached process. Returns the process id.
ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline);

/// Run the `commandline` with `working_dir` as detached process. Returns the process id.
ALBERT_EXPORT long long runDetachedProcess(const QStringList &commandline, const QString &working_dir);

/// Tries to create a directory at `path`.
/// Throws std::runtime_error if the directory could not be created.
/// This is a utility function for use with the *Location functions.
ALBERT_EXPORT void tryCreateDirectory(const std::filesystem::path &path);

/// Returns a QString representation of `path`.
ALBERT_EXPORT QString toQString(const std::filesystem::path &path);

}
