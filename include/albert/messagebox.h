// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <QMessageBox>
#include <albert/export.h>

namespace albert::util
{

///
/// Shows a question message box containing _text_ with the given _buttons_ and _default_button_.
///
/// The message box modal will appear modal to the main window and have the app name as title.
///
/// Returns the button pressed by the user.
///
ALBERT_EXPORT QMessageBox::StandardButton
question(const QString &text,
         QMessageBox::StandardButtons buttons
            = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
         QMessageBox::StandardButton default_button = QMessageBox::NoButton);

///
/// Shows an information message box containing _text_ with the given _buttons_ and _default_button_.
///
/// The message box modal will appear modal to the main window and have the app name as title.
///
/// Returns the button pressed by the user.
///
ALBERT_EXPORT QMessageBox::StandardButton
information(const QString &text,
            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton default_button = QMessageBox::NoButton);

///
/// Shows a warning message box containing _text_ with the given _buttons_ and _default_button_.
///
/// The message box modal will appear modal to the main window and have the app name as title.
///
/// Returns the button pressed by the user.
///
ALBERT_EXPORT QMessageBox::StandardButton
warning(const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton default_button = QMessageBox::NoButton);

///
/// Shows a critical message box containing _text_ with the given _buttons_ and _default_button_.
///
/// The message box modal will appear modal to the main window and have the app name as title.
///
/// Returns the button pressed by the user.
///
ALBERT_EXPORT QMessageBox::StandardButton
critical(const QString &text,
         QMessageBox::StandardButtons buttons = QMessageBox::Ok,
         QMessageBox::StandardButton default_button = QMessageBox::NoButton);

}
