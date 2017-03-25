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
#include <QWidget>
#include "ui_configwidget.h"

namespace ChromeBookmarks {

class ConfigWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = 0);
    ~ConfigWidget();
    Ui::ConfigWidget ui;

private:
    void onButton_EditPath();

signals:
    void requestEditPath(const QString&);
};
}
