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
#include <QPushButton>
class QPropertyAnimation;
class QSvgRenderer;

class SettingsButton final : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int angle MEMBER angle_)

public:

    SettingsButton(QWidget *parent = 0);
    ~SettingsButton();

protected:

    void hideEvent(QHideEvent * event) override;
    void showEvent(QShowEvent * event) override;

private:

    void paintEvent(QPaintEvent *event) override;

    int angle_;
    QPropertyAnimation *animation_;
    QSvgRenderer *svgRenderer_;

};
