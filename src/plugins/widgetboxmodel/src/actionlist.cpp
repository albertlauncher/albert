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

#include <QKeyEvent>
#include <QPainter>
#include "actionlist.h"

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
            keyPressEvent(keyEvent);
            return false;
        }
    }
    return false;
}



/** ***************************************************************************/
void ActionList::ActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const {

    painter->save();

    QStyleOptionViewItem option = options;
    initStyleOption(&option, index);

    // Draw selection
    option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    // Draw text
    painter->setFont(option.font);
    QString text = QFontMetrics(option.font).elidedText(index.data(Qt::DisplayRole).toString(), option.textElideMode, option.rect.width());
    option.widget->style()->drawItemText(painter,
                                         option.rect,
                                         Qt::AlignCenter|Qt::AlignHCenter,
                                         option.palette,
                                         option.state & QStyle::State_Enabled,
                                         text,
                                         (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::WindowText);
    painter->restore();
}
