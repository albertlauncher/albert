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
#include <QString>
#include <QWidget>
#include <QAbstractItemModel>
#include "core_globals.h"

namespace Core {

class EXPORT_CORE Frontend : public QWidget
{
    Q_OBJECT

public:

    Frontend(QString id, QWidget* parent = nullptr)
        : QWidget(parent), id(id) {}
    virtual ~Frontend() {}

    const QString id;

    virtual void setInput(const QString&) = 0;
    virtual void setModel(QAbstractItemModel *) = 0;
    virtual QWidget *widget(QWidget *parent) = 0;

    void toggleVisibility() { setVisible(!isVisible()); }

signals:

    void widgetShown();
    void widgetHidden();
    void inputChanged(QString qry);
    void settingsWidgetRequested();

};

}
