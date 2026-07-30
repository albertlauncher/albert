// Copyright (c) 2026-2026 Manuel Schneider

#include "hotkey.h"
#include <QHotkey>
#ifdef HAVE_WAYLAND_HOTKEY
#include "waylandhotkey.h"
#endif
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
    QKeyCombination key_combination;
    unique_ptr<QHotkey> hotkey;
#ifdef HAVE_WAYLAND_HOTKEY
    unique_ptr<WaylandHotkey> wayland_hotkey;
#endif
};

Hotkey::Hotkey(QKeyCombination kc) :
    d(make_unique<Private>())
{
    d->key_combination = kc;

#ifdef HAVE_WAYLAND_HOTKEY
    if (WaylandHotkey::isPlatformSupported())
    {
        if (auto hk = WaylandHotkey::grab(kc))
        {
            d->wayland_hotkey = ::move(hk.value());
            connect(d->wayland_hotkey.get(), &WaylandHotkey::activated, this, &Hotkey::activated);
            connect(d->wayland_hotkey.get(), &WaylandHotkey::revoked, this, &Hotkey::revoked);
            return;
        }
        else
            throw runtime_error(u"Failed to register hotkey '%1': %2"_s
                                    .arg(toString(kc, NativeText), hk.error())
                                    .toStdString());
    }
#endif

    d->hotkey = make_unique<QHotkey>();
    d->hotkey->setShortcut(kc);

    if (!d->hotkey->setRegistered(true))
        throw runtime_error(u"Failed to register hotkey '%1'"_s
                                .arg(toString(kc, NativeText))
                                .toStdString());

    connect(d->hotkey.get(), &QHotkey::activated, this, &Hotkey::activated);
}

Hotkey::~Hotkey() {}

QKeyCombination Hotkey::keyCombination() const { return d->key_combination; }

QString Hotkey::nativeString() const { return toString(keyCombination(), NativeText); }

QString Hotkey::portableString() const { return toString(keyCombination(), PortableText); }

bool Hotkey::isPlatformSupported()
{
#ifdef HAVE_WAYLAND_HOTKEY
    if (WaylandHotkey::isPlatformSupported())
        return true;
#endif
    return QHotkey::isPlatformSupported();
}

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
