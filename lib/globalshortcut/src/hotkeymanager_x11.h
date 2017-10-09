// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <QDebug>
#include <set>
#include <QAbstractNativeEventFilter>

struct _XDisplay;
typedef _XDisplay Display;

namespace GlobalShortcut {

class HotkeyManagerPrivate final : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:

    HotkeyManagerPrivate(QObject* parent = 0);
    ~HotkeyManagerPrivate();

    bool registerNativeHotkey(quint32 hk);
    void unregisterNativeHotkey(quint32 hk);

private:

    bool nativeEventFilter(const QByteArray&, void*, long*) override;
    std::set<uint> nativeKeycodes(uint QtKey);
    uint nativeModifiers(uint QtKbdMods);

    std::set<std::pair<uint,uint>> grabbedKeys;
    std::set<uint> offendingMasks;

    struct Masks {
        unsigned int alt;
        unsigned int meta;
        unsigned int numlock;
        unsigned int super;
        unsigned int hyper;
        unsigned int iso_level3_shift;
    } masks;

signals:

     void hotKeyPressed();
};

}
