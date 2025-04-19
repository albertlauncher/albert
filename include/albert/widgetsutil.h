// SPDX-FileCopyrightText: 2025 Manuel Schneider

#pragma once
#include <albert/export.h>
#include <QString>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>

namespace albert::util
{

#ifdef QCHECKBOX_H

template<typename T, typename GET, typename SET>
static void bind(QCheckBox *w, T *t, GET get, SET set)
{
    w->setChecked((t->*get)());
    QObject::connect(w, &QCheckBox::toggled, t, set);
}

template<typename T, typename GET, typename SET, typename SIG>
static void bind(QCheckBox *w, T *t, GET get, SET set, SIG sig)
{
    bind(w, t, get, set);
    QObject::connect(t, sig, w, &QCheckBox::setChecked);
}

#endif

#ifdef QLINEEDIT_H

template<typename T, typename GET, typename SET>
static void bind(QLineEdit *w, T *t, GET get, SET set)
{
    w->setText((t->*get)());
    QObject::connect(w, &QLineEdit::editingFinished,
                     [w, t, set]() { (t->*set)(w->text()); });
}

template<typename T, typename GET, typename SET, typename SIG>
static void bind(QLineEdit *w, T *t, GET get, SET set, SIG sig)
{
    bind(w, t, get, set);
    QObject::connect(t, sig, w, &QLineEdit::setText);
}

#endif

#ifdef QSPINBOX_H

template<typename T, typename GET, typename SET>
static void bind(QSpinBox *w, T *t, GET get, SET set)
{
    w->setValue((t->*get)());
    QObject::connect(w, QOverload<int>::of(&QSpinBox::valueChanged), t, set);
}

template<typename T, typename GET, typename SET, typename SIG>
static void bind(QSpinBox *w, T *t, GET get, SET set, SIG sig)
{
    bind(w, t, get, set);
    QObject::connect(t, sig, w, &QSpinBox::setValue);
}

template<typename T, typename GET, typename SET>
static void bind(QDoubleSpinBox *w, T *t, GET get, SET set)
{
    w->setValue((t->*get)());
    QObject::connect(w, QOverload<double>::of(&QDoubleSpinBox::valueChanged), t, set);
}

template<typename T, typename GET, typename SET, typename SIG>
static void bind(QDoubleSpinBox *w, T *t, GET get, SET set, SIG sig)
{
    bind(w, t, get, set);
    QObject::connect(t, sig, w, &QDoubleSpinBox::setValue);
}

#endif

}
