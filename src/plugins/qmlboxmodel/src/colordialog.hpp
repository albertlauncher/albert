// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QColorDialog>

class ColorDialog : public QColorDialog
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged USER true)
public:
    ColorDialog(QWidget * parent = 0) : QColorDialog(parent){
        setOptions(QColorDialog::ShowAlphaChannel);
        connect(this, &QColorDialog::currentColorChanged,
                this, &ColorDialog::colorChanged);
    }

    QColor color(){ return currentColor(); }
    void setColor(const QColor& c){ setCurrentColor(c); }
signals:
    void colorChanged(const QColor &color);
};
