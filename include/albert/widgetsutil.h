// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QString>
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

///
/// Binds a property of type `bool` of _object_ to _checkbox_.
///
/// Initializes _checkbox_ using _get_ and connects the `toggled` signal to _set_.
///
template<typename T, typename GET, typename SET>
static void bind(QCheckBox *checkbox, T *object, GET get, SET set)
{
    checkbox->setChecked((object->*get)());
    QObject::connect(checkbox, &QCheckBox::toggled, object, set);
}

///
/// Binds a property of type `bool` of _object_ to _checkbox_.
///
/// Initializes _checkbox_ using _get_,
/// connects the `toggled` signal to _set_ and
/// connects the signal _sig_ to `setChecked`.
///
template<typename T, typename GET, typename SET, typename SIG>
static void bind(QCheckBox *checkbox, T *object, GET get, SET set, SIG sig)
{
    bind(checkbox, object, get, set);
    QObject::connect(object, sig, checkbox, &QCheckBox::setChecked);
}

///
/// Binds a property of type `QString` of _object_ to _lineedit_.
///
/// Initializes _lineedit_ using _get_ and
/// connects the `editingFinished` signal to _set_.
///
template<typename T, typename GET, typename SET>
static void bind(QLineEdit *lineedit, T *object, GET get, SET set)
{
    lineedit->setText((object->*get)());
    QObject::connect(lineedit, &QLineEdit::editingFinished,
                     object, [lineedit, object, set] { (object->*set)(lineedit->text()); });
}

///
/// Binds a property of type `QString` of _object_ to _lineedit_.
///
/// Initializes _lineedit_ using _get_,
/// connects the `editingFinished` signal to _set_ and
/// connects the signal _sig_ to `setText`.
///
template<typename T, typename GET, typename SET, typename SIG>
static void bind(QLineEdit *lineedit, T *object, GET get, SET set, SIG sig)
{
    bind(lineedit, object, get, set);
    QObject::connect(object, sig, lineedit, &QLineEdit::setText);
}

///
/// Binds a property of type `int` of _object_ to _spinbox_.
///
/// Initializes _spinbox_ using _get_ and
/// connects the `valueChanged` signal to _set_.
///
template<typename T, typename GET, typename SET>
static void bind(QSpinBox *spinbox, T *object, GET get, SET set)
{
    spinbox->setValue((object->*get)());
    QObject::connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged), object, set);
}

///
/// Binds a property of type `int` of _object_ to _spinbox_.
///
/// Initializes _spinbox_ using _get_,
/// connects the `valueChanged` signal to _set_ and
/// connects the signal _sig_ to `setValue`.
///
template<typename T, typename GET, typename SET, typename SIG>
static void bind(QSpinBox *spinbox, T *object, GET get, SET set, SIG sig)
{
    bind(spinbox, object, get, set);
    QObject::connect(object, sig, spinbox, &QSpinBox::setValue);
}

///
/// Binds a property of type `double` of _object_ to _spinbox_.
///
/// Initializes _spinbox_ using _get_ and
/// connects the `valueChanged` signal to _set_.
///
template<typename T, typename GET, typename SET>
static void bind(QDoubleSpinBox *spinbox, T *object, GET get, SET set)
{
    spinbox->setValue((object->*get)());
    QObject::connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), object, set);
}

///
/// Binds a property of type `double` of _object_ to _spinbox_.
///
/// Initializes _spinbox_ using _get_,
/// connects the `valueChanged` signal to _set_ and
/// connects the signal _sig_ to `setValue`.
///
template<typename T, typename GET, typename SET, typename SIG>
static void bind(QDoubleSpinBox *spinbox, T *object, GET get, SET set, SIG sig)
{
    bind(spinbox, object, get, set);
    QObject::connect(object, sig, spinbox, &QDoubleSpinBox::setValue);
}

}
