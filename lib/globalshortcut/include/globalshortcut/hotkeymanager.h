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
#include <QObject>
#include <QSet>
#include <memory>
#include "globalshortcut_globals.h"

namespace GlobalShortcut {

class HotkeyManagerPrivate;

class EXPORT_GLOBALSHORTCUT HotkeyManager final : public QObject
{
    Q_OBJECT

public:
    HotkeyManager(QObject *parent = 0);
    ~HotkeyManager();

    bool registerHotkey(const QString&);
    bool registerHotkey(const QKeySequence&);
    bool registerHotkey(const int);
    bool unregisterHotkey(const QString&);
    bool unregisterHotkey(const QKeySequence&);
    void unregisterHotkey(const int);
    QSet<int> hotkeys() {return hotkeys_;}
    void enable(){ setEnabled(true); }
    void disable(){ setEnabled(false); }
    void setEnabled(bool enabled = true){enabled_ = enabled;}
    bool isEnabled() const {return enabled_;}

private:
    void onHotkeyPressed();

    bool enabled_;
    QSet<int> hotkeys_;

    std::unique_ptr<HotkeyManagerPrivate> d;

signals:
	void hotKeyPressed();
};

}
