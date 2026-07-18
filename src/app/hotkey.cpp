// Copyright (c) 2026-2026 Manuel Schneider

#include "hotkey.h"
#include <QHotkey>
using enum QKeySequence::SequenceFormat;
using namespace Qt::StringLiterals;
using namespace std;

namespace {

static inline QString toString(QKeyCombination kc, QKeySequence::SequenceFormat fmt)
{ return QKeySequence(kc).toString(fmt); }

static inline QKeyCombination toKeyCombination(const QString &s)
{ return QKeySequence(s)[0]; }

}

class Hotkey::Private
{
public:
    QHotkey hotkey;
};

Hotkey::Hotkey(QKeyCombination kc) :
    d(make_unique<Private>())
{
    d->hotkey.setShortcut(kc);

    if (!d->hotkey.setRegistered(true))
        throw runtime_error(u"Failed to register hotkey '%1'"_s
                                .arg(QKeySequence(kc).toString(QKeySequence::NativeText))
                                .toStdString());

    connect(&d->hotkey, &QHotkey::activated, this, &Hotkey::activated);
}

Hotkey::~Hotkey() {}

QKeyCombination Hotkey::keyCombination() const { return d->hotkey.shortcut()[0]; }

QString Hotkey::nativeString() const { return toString(keyCombination(), NativeText); }

QString Hotkey::portableString() const { return toString(keyCombination(), PortableText); }

bool Hotkey::isPlatformSupported() { return QHotkey::isPlatformSupported(); }

expected<unique_ptr<Hotkey>, QString> Hotkey::grab(QKeyCombination kc)
{
    try{
        return make_unique<Hotkey>(kc);
    } catch (const std::exception &e) {
        return unexpected(QString::fromUtf8(e.what()));
    }
}

expected<std::unique_ptr<Hotkey>, QString> Hotkey::grab(const QString &s)
{ return grab(toKeyCombination(s)); }
