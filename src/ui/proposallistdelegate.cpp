// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#include "proposallistdelegate.h"

#include <QPainter>

/**************************************************************************/
ProposalListDelegate::ProposalListDelegate(Qt::KeyboardModifiers mods)
{
	switch (mods) {
	case Qt::NoModifier: _role = Qt::UserRole;break;
	case Qt::ControlModifier: _role = Qt::UserRole+1;break;
	case Qt::MetaModifier: _role = Qt::UserRole+2;break;
	case Qt::AltModifier: _role = Qt::UserRole+3;break;
	default:break;
	}
}

/**************************************************************************/
void ProposalListDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
//	QStyledItemDelegate::paint(painter, option, index);
	painter->save();
	QStyle *style = option.widget->style();
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

	/*
	 *  a = rect.height
	 * fm(x) := fontmetrics of x
	 * DR := DisplayRole
	 * UR := UserRole
	 *             a                            rect.width-a
	 *  +---------------------+------------------------------------------------+
	 *  |                     |                                                |
	 *  |   +-------------+   |                                                |
	 *  |   |             |   |                                                |
	 *  |   |             |   |a*fm(DR)/(fm(DR)+fm(UR))            DisplayRole |
	 * a|   |     icon    |   |                                                |
	 *  |   |             |   |                                                |
	 *  |   |             |   +------------------------------------------------+
	 *  |   |             |   |                                                |
	 *  |   +-------------+   |a*fm(UR)/(fm(DR)+fm(UR))             UserRole+x |
	 *  |                     |                                                |
	 * +-----------------------------------------------------------------------+
	 */

	QRect contentsRect = style->subElementRect(QStyle::SE_ItemViewItemText,
											   &option,
											   option.widget);
	int a = contentsRect.height();

	/* Draw icon */
	QRect iconRect(contentsRect.topLeft(), option.decorationSize);
	iconRect.translate( (a-option.decorationSize.width())/2, (a-option.decorationSize.height())/2);
	painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QIcon>().pixmap(option.decorationSize));

	/* Draw name */
	QRect DRTextRect(contentsRect.adjusted(
		a,0,
		0,-a*12/(option.fontMetrics.height()+12)) // TODO
	);
	DRTextRect.adjust(3,-2,0,-2);  // Empirical
	QFont font = option.font;
	QString text = QFontMetrics(font).elidedText(
				index.data(Qt::DisplayRole).toString(),
				option.textElideMode,
				DRTextRect.width());
	painter->setFont(font);
	painter->drawText(DRTextRect, Qt::AlignVCenter|Qt::AlignLeft, text);

	/* Draw the infotext */
	QRect URTextRect(contentsRect.adjusted(
		a,DRTextRect.height(),
		0,0)
	);
	URTextRect.adjust(3,-4,0,-4);  // Empirical
	font.setPixelSize(12);
	text = QFontMetrics(font).elidedText(
				index.data(_role).toString(),
				option.textElideMode,
				URTextRect.width());
	painter->setFont(font);
	painter->drawText(URTextRect, Qt::AlignVCenter|Qt::AlignLeft, text);

	painter->restore();
}
