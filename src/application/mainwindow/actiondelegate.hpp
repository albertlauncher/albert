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
#include <QStyledItemDelegate>
#include <QPainter>
#include "roles.hpp"

class ActionDelegate final : public QStyledItemDelegate
{
public:
    ActionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override {

        painter->save();

        QStyleOptionViewItemV4 option = options;
        initStyleOption(&option, index);

        // Draw selection
        option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

        // Draw text
        QRect textRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
        painter->setFont(option.font);

        // Draw text
        QString text = QFontMetrics(option.font).elidedText(index.data(Roles::Text).toString(), option.textElideMode, textRect.width());
        option.widget->style()->drawItemText(painter, textRect, Qt::AlignCenter, option.palette, option.state & QStyle::State_Enabled, text, QPalette::WindowText);
        //    painter->drawText(textRect, Qt::AlignTop|Qt::AlignLeft, text);

        painter->restore();
    }
};
