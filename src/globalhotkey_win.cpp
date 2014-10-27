// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#include "globalhotkey_p.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
GlobalHotkey::GlobalHotkeyPrivate::GlobalHotkeyPrivate(QObject *parent)
    : QObject(parent)
{

    QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
}

/**************************************************************************/
bool GlobalHotkey::GlobalHotkeyPrivate::registerNativeHotkey(const Hotkey& hk)
{

}

/**************************************************************************/
void GlobalHotkey::GlobalHotkeyPrivate::unregisterNativeHotkeys()
{

}

/**************************************************************************/
bool GlobalHotkey::GlobalHotkeyPrivate::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if (eventType == "xcb_generic_event_t") {

    }
    return false;
}
