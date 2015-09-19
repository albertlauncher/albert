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
#include <QPushButton>
#include <QIcon>
#include <QRect>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionButton>

class SettingsButton final : public QPushButton
{
    Q_OBJECT
public:
    explicit SettingsButton(QWidget *parent = 0) :
        QPushButton(parent){}

private:
    void paintEvent(QPaintEvent *event) override
    {
        QPushButton::paintEvent(event);
        QStyleOptionButton option;
        option.initFrom(this);
        QRect r = this->rect();

        // Prepare miage in pixmap using mask
        QPixmap pm(r.size());
        pm.fill(Qt::transparent);
        pm.fill(option.palette.windowText().color());
        QIcon w(":gear");
        QPainter ppm(&pm);
        ppm.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        ppm.drawPixmap(0,0,w.pixmap(r.size()));

        // Draw pixmap on button
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawPixmap(0,0,pm);
    }
};
