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
#include "itemdelegate.h"

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const {
    QStyleOptionViewItemV4 option = options;
    initStyleOption(&option, index);

    //	QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    QStyle *style = option.widget->style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
//		QRect contentsRect = style->subElementRect(QStyle::SE_ItemViewItemText,
//												   &option,
//												   option.widget);

    /* Draw icon */
//		QRect iconRect(contentsRect.topLeft(), option.decorationSize);
//		iconRect.translate( (a-option.decorationSize.width())/2, (a-option.decorationSize.height())/2);
    QRect iconRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, option.widget);
    painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QIcon>().pixmap(option.decorationSize));

    /* Drawing text differs dependent on the mode and selection */
    if (showInfo && (!showForSelectedOnly || option.state.testFlag(QStyle::State_Selected)) )
    {
        /*
         * fm(x) := fontmetrics of x
         * DR := DisplayRole
         * TR := ToolTipRole
         *  +---------------------+----------------------------------------+
         *  |                     |                                        |
         *  |   +-------------+   |                                        |
         *  |   |             |   |                                        |
         *  |   |             |   |a*fm(DR)/(fm(DR)+fm(TR))    DisplayRole |
         * a|   |     icon    |   |                                        |
         *  |   |             |   |                                        |
         *  |   |             |   +----------------------------------------+
         *  |   |             |   |                                        |
         *  |   +-------------+   |a*fm(TR)/(fm(DR)+fm(TR))  ToolTipRole+x |
         *  |                     |                                        |
         * +---------------------------------------------------------------+
         */

        QRect DisplayRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
        DisplayRect.adjust(3,0,0,-5);  // Empirical
        QFont font = option.font;
        painter->setFont(font);
        QString text = QFontMetrics(font).elidedText(
                    index.data(Qt::DisplayRole).toString(),
                    option.textElideMode,
                    DisplayRect.width());
        painter->drawText(DisplayRect, Qt::AlignTop|Qt::AlignLeft, text);
        font.setPixelSize(12);
        painter->setFont(font);
        text = QFontMetrics(font).elidedText(
                    index.data(Qt::ToolTipRole)
                    .toString(),
                    option.textElideMode,
                    DisplayRect.width());
        painter->drawText(DisplayRect, Qt::AlignBottom|Qt::AlignLeft, text);
    }
    else
    {
        QRect DisplayRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
        QString text = QFontMetrics(option.font).elidedText(
                    index.data(Qt::DisplayRole).toString(),
                    option.textElideMode,
                    DisplayRect.width());
        painter->drawText(DisplayRect, Qt::AlignVCenter|Qt::AlignLeft, text);
    }
    painter->restore();
}
