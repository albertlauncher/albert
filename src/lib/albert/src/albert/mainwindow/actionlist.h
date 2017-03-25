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
#include <QEvent>
#include "resizinglist.h"
#include <QStyledItemDelegate>

/** ***************************************************************************/
class ActionList final : public ResizingList
{
    Q_OBJECT
    class ActionDelegate;

public:

    ActionList(QWidget *parent = 0);

private:

    bool eventFilter(QObject*, QEvent *event) override;

};



/** ***************************************************************************/
class ActionList::ActionDelegate final : public QStyledItemDelegate
{
public:

    ActionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
};
