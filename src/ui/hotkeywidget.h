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

#ifndef HOTKEYWIDGET_H
#define HOTKEYWIDGET_H

#include <QLabel>

class HotkeyWidget : public QLabel
{
	Q_OBJECT

public:
	explicit HotkeyWidget(QWidget *parent = 0);

private:
	bool _settingHotkey;

	QString keyKomboToString(Qt::KeyboardModifiers mod, int key);
	void grabAll();
	void releaseAll();

protected:
	void mousePressEvent (QMouseEvent*)  override;
	void keyPressEvent (QKeyEvent *) override;
	void keyReleaseEvent ( QKeyEvent* ) override;
};

#endif // HOTKEYWIDGET_H
