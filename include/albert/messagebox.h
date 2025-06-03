// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <albert/export.h>

namespace albert::util
{

/// Shows a question message box containing _text_ with Yes and No buttons.
/// The message box modal will appear modal to the main window and have the app name as title.
/// Returns `true` if the user pressed yes, `false` otherwise.
ALBERT_EXPORT bool question(const QString &text);

/// Shows an information message box containing _text_ with a single Ok button.
/// The message box modal will appear modal to the main window and have the app name as title.
ALBERT_EXPORT void information(const QString &text);

/// Shows a warning message box containing _text_ with a single Ok button.
/// The message box modal will appear modal to the main window and have the app name as title.
ALBERT_EXPORT void warning(const QString &text);

/// Shows a critical message box containing _text_ with a single Ok button.
/// The message box modal will appear modal to the main window and have the app name as title.
ALBERT_EXPORT void critical(const QString &text);

}
