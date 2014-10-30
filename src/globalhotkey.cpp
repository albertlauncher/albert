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

#include "globalhotkey.h"
#include "globalhotkey_p.h"

#include <QKeySequence>

/**************************************************************************/
GlobalHotkey::GlobalHotkey(QObject *parent) :
	QObject(parent)
{
	_impl = new GlobalHotkeyPrivate;
	connect(_impl, SIGNAL(hotKeyPressed()), this, SLOT(onHotkeyPressed()));
	_enabled = true;
	_hotkey = 0;
}

/**************************************************************************/
GlobalHotkey::~GlobalHotkey()
{
	delete _impl;
}

/**************************************************************************/
bool GlobalHotkey::setHotkey(const QString &hk)
{
	return setHotkey(QKeySequence(hk));
}

/**************************************************************************/
bool GlobalHotkey::setHotkey(const QKeySequence &hk)
{
	if (hk.count() != 1)
		return false;
	return setHotkey(hk[0]);
}

/**************************************************************************/
bool GlobalHotkey::setHotkey(const int hk)
{
	// Unregister other hotkeys before registering new ones
	unsetHotkey();

	//TODO make this capable of multiple key, so that the old one does not have to be unregistered whil registereing another
	if (_impl->registerNativeHotkey(hk)) {
		_hotkey = hk;
		return true;
	}
	return false;
}

/**************************************************************************/
int GlobalHotkey::hotkey()
{
	return _hotkey;
}


/**************************************************************************/
void GlobalHotkey::unsetHotkey()
{
	_impl->unregisterNativeHotkeys();
	_hotkey = 0;
}
/**************************************************************************/
bool GlobalHotkey::isEnabled() const
{
	return _enabled;
}

/**************************************************************************/
void GlobalHotkey::setEnabled(bool enabled)
{
	_enabled = enabled;
}

/**************************************************************************/
void GlobalHotkey::onHotkeyPressed()
{
	if (_enabled)
		emit hotKeyPressed();
}

