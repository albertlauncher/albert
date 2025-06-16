// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <albert/export.h>
class QWidget;
class QString;

namespace albert::util
{

/// Shows a question message box with Yes and No buttons.
/// The title of the message box is set to the application name and the message to _text_.
/// The message box will appear modal to _parent_ or the main window if undefined.
/// Returns \c true if the user pressed yes, \c false otherwise.
ALBERT_EXPORT bool question(const QString &text, QWidget *parent = nullptr);

/// Shows an information message box with a single Ok button.
/// The title of the message box is set to the application name and the message to _text_.
/// The message box will appear modal to _parent_ or the main window if undefined.
ALBERT_EXPORT void information(const QString &text, QWidget *parent = nullptr);

/// Shows a warning message box with a single Ok button.
/// The title of the message box is set to the application name and the message to _text_.
/// The message box will appear modal to _parent_ or the main window if undefined.
ALBERT_EXPORT void warning(const QString &text, QWidget *parent = nullptr);

/// Shows a critical message box with a single Ok button.
/// The title of the message box is set to the application name and the message to _text_.
/// The message box will appear modal to _parent_ or the main window if undefined.
ALBERT_EXPORT void critical(const QString &text, QWidget *parent = nullptr);

}
