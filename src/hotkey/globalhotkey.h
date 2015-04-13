// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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
#include <QObject>
#include <QSet>
#include "singleton.h"

#define gHotkeyManager GlobalHotkey::instance()

class GlobalHotkey final : public QObject, public Singleton<GlobalHotkey>
{
    Q_OBJECT
    friend class Singleton<GlobalHotkey>;
    class GlobalHotkeyPrivate;

public:
    virtual   ~GlobalHotkey();
    bool      registerHotkey(const QString&);
    bool      registerHotkey(const QKeySequence&);
    bool      registerHotkey(const int);
    bool      unregisterHotkey(const QString&);
    bool      unregisterHotkey(const QKeySequence&);
    void      unregisterHotkey(const int);
    QSet<int> hotkeys() {return _hotkeys;}
    void      enable(){ setEnabled(true); }
    void      disable(){ setEnabled(false); }
    void      setEnabled(bool enabled = true){_enabled = enabled;}
    bool      isEnabled() const {return _enabled;}

private:
    explicit  GlobalHotkey(QObject *parent = 0);
    void      onHotkeyPressed();

    bool       _enabled;
    QSet<int>  _hotkeys;
    GlobalHotkeyPrivate* _impl;


signals:
	void hotKeyPressed();
};
