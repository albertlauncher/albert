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

#ifndef GLOBALHOTKEY_H
#define GLOBALHOTKEY_H

#include <QObject>

class GlobalHotkey : public QObject
{
	Q_OBJECT
	class GlobalHotkeyPrivate;

private:
	bool _enabled;
	int  _hotkey;
	GlobalHotkeyPrivate* _impl;

public:
	explicit GlobalHotkey(QObject *parent = 0);
	virtual ~GlobalHotkey();

	bool registerHotkey(const QString&);
	bool registerHotkey(const QKeySequence&);
	bool registerHotkey(const int);
	void unregisterHotkey();
	int hotkey();
	bool isEnabled() const;

signals:
	void hotKeyPressed();
	void hotKeyChanged(int);

private slots:
	void onHotkeyPressed();

public slots:
	void setEnabled(bool enabled = true);
};

#endif // GLOBALHOTKEY_H
