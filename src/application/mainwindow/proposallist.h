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
#include <QEvent>
#include "resizinglist.h"
#include <QStyledItemDelegate>

/** ***************************************************************************/
class ProposalList final : public ResizingList
{
    Q_OBJECT
    class ItemDelegate;

public:

    ProposalList(QWidget *parent = 0);

    bool displayIcons() const;
    void setDisplayIcons(bool value);

private:

    bool eventFilter(QObject*, QEvent *event) override;

    ItemDelegate *delegate_;
};



/** ***************************************************************************/
class ProposalList::ItemDelegate final : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent), drawIcon(true) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

    bool drawIcon;
};
