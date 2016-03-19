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
#include <QLineEdit>
#include <list>
using std::list;
#include "settingsbutton.h"

class InputLine final : public QLineEdit
{
    Q_OBJECT
public:
    explicit InputLine(QWidget *parent = 0);
    ~InputLine();

    void clearHistory() { lines_.clear(); }
    void clear();

    SettingsButton   *settingsButton_;

private:
    void keyPressEvent(QKeyEvent*) override;
    void wheelEvent(QWheelEvent *) override;
    void resizeEvent(QResizeEvent*) override;

    void resetIterator();
    void next();
    void prev();


    list<QString> lines_; // NOTE fix this in 5.6, qt has no reverse iterators https://codereview.qt-project.org/#/c/109850/
    list<QString>::const_reverse_iterator currentLine_;

    static constexpr const char * SETTINGS_SHORTCUT = "Alt+,"; // FIXME
};
