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
#include <QMimeType>
#include <QMimeDatabase>
#include <QGuiApplication>
#include <QStylePainter>
#include <QApplication>
#include <QDebug>





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


#include <QPushButton>
/**************************************************************************/
void ProposalListDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
//	QStyledItemDelegate::paint(painter, option, index);

	painter->save();

	QStyle *style = option.widget->style();
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
//	style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

	/* a = rect.height
	 * fm(x) := fontmetrics of x
	 * DR := DisplayRole
	 * UR := UserRole
	 *             a                            rect.width-a
	 *  +---------------------+------------------------------------------------+
	 *  |                     |                                                |
	 *  |   +-------------+   |                                                |
	 *  |   |             |   |                                                |
	 *  |   |             |   |a*fm(DR)/(fm(rd)+fm(UR))            DisplayRole |
	 * a|   |     icon    |   |                                                |
	 *  |   |             |   |                                                |
	 *  |   |             |   +------------------------------------------------+
	 *  |   |             |   |                                                |
	 *  |   +-------------+   |a*fm(UR)/(fm(rd)+fm(UR))             UserRole+x |
	 *  |                     |                                                |
	 * +-----------------------------------------------------------------------+
	 */

	int a = option.rect.height();

	/* Draw icon */
	QRect iconRect(option.rect.topLeft(), option.decorationSize);
	iconRect.translate( (a-option.decorationSize.width())/2,
						(a-option.decorationSize.height())/2);
	painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QIcon>().pixmap(option.decorationSize));


	/* Draw texts */

	// Calculate the text rects
	QRect DRTextRect(option.rect.adjusted(
		a,0,
		0,-a*12/(option.fontMetrics.height()+12)) // TODO
	);

	QRect URTextRect(option.rect.adjusted(
		a,DRTextRect.height(),
		0,0)
	);

	// Draw name
	QFont font = option.font;
	QString text = index.data(Qt::DisplayRole).toString();
	text = QFontMetrics(font).elidedText(text, Qt::ElideRight, DRTextRect.width());
	painter->setFont(font);
	painter->drawText(DRTextRect, Qt::AlignVCenter|Qt::AlignLeft, text);

	//Draw the infotext
	font.setPixelSize(12);
	text = index.data(_role).toString();
	text = QFontMetrics(font).elidedText(text, Qt::ElideMiddle, URTextRect.width());
	painter->setFont(font);
	painter->drawText(URTextRect, Qt::AlignVCenter|Qt::AlignLeft, text);

	painter->restore();
}


