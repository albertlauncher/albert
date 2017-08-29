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

#include "globalshortcut/hotkeymanager.h"
#if defined __linux__
#include "hotkeymanager_x11.h"
#elif defined __APPLE__
#elif defined _WIN32
#include "hotkeymanager_win.h"
#endif
#include <QKeySequence>

/** ***************************************************************************/
GlobalShortcut::HotkeyManager::HotkeyManager(QObject *parent) :
    QObject(parent), d(new HotkeyManagerPrivate) {
    connect(d.get(), &HotkeyManagerPrivate::hotKeyPressed, this, &HotkeyManager::onHotkeyPressed);
    enabled_ = true;
}

/** ***************************************************************************/
GlobalShortcut::HotkeyManager::~HotkeyManager() {

}

/** ***************************************************************************/
bool GlobalShortcut::HotkeyManager::registerHotkey(const QString &hk) {
	return registerHotkey(QKeySequence(hk));
}

/** ***************************************************************************/
bool GlobalShortcut::HotkeyManager::registerHotkey(const QKeySequence &hk) {
	if (hk.count() != 1)
		return false;
	return registerHotkey(hk[0]);
}

/** ***************************************************************************/
bool GlobalShortcut::HotkeyManager::registerHotkey(const int hk) {
    if (hotkeys_.contains(hk))
        return true;
    if (d->registerNativeHotkey(hk)) {
        hotkeys_.insert(hk);
		return true;
	}
    return false;
}

/** ***************************************************************************/
bool GlobalShortcut::HotkeyManager::unregisterHotkey(const QString &hk) {
    return unregisterHotkey(QKeySequence(hk));
}

/** ***************************************************************************/
bool GlobalShortcut::HotkeyManager::unregisterHotkey(const QKeySequence &hk) {
    if (hk.count() != 1)
        return false;
    unregisterHotkey(hk[0]);
    return true;
}

/** ***************************************************************************/
void GlobalShortcut::HotkeyManager::unregisterHotkey(const int hk) {
    if (!hotkeys_.contains(hk))
        return;
    d->unregisterNativeHotkey(hk);
    hotkeys_.remove(hk);
}

/** ***************************************************************************/
void GlobalShortcut::HotkeyManager::onHotkeyPressed() {
    if (enabled_)
		emit hotKeyPressed();
}

