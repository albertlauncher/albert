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

#include <QKeyEvent>
#include <QPainter>
#include "actionlist.h"
#include "roles.hpp"

/** ***************************************************************************/
ActionList::ActionList(QWidget *parent) : ResizingList(parent) {
    setItemDelegate(new ActionDelegate);
}



/** ***************************************************************************/
bool ActionList::eventFilter(QObject*, QEvent *event) {

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {

        // Navigation
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Home:
        case Qt::Key_End:
        // Activation
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if ( keyEvent->modifiers() == Qt::NoModifier )
                keyPressEvent(keyEvent);
            return false;
        }
    }
    return false;
}



/** ***************************************************************************/
void ActionList::ActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const {

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
