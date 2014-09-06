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

#

/**************************************************************************//**
 * @brief ProposalListDelegate::ProposalListDelegate
 */
ProposalListDelegate::ProposalListDelegate()
{
}

/**************************************************************************//**
 * @brief ProposalListDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void ProposalListDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	//	EnterText = Qt::UserRole;
	//	Mod1Text = Qt::UserRole+1;
	//	Mod2Text = Qt::UserRole+2;
	//	InfoText = Qt::UserRole+3;

	QString elided;
	QFont font = option.font;
	font.setPixelSize(12); // Size for the infotext

	// Draw selection
	if(option.state & QStyle::State_Selected){
		// Draw a selection background
		QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomRight());
		gradient.setColorAt(0, option.widget->palette().color(QPalette::Window).lighter(120)  );
		gradient.setColorAt(1, option.widget->palette().color(QPalette::Window));
		painter->fillRect(option.rect, gradient);

		//Draw the infotext
		elided = QFontMetrics(font).elidedText(index.data(Qt::UserRole).toString(), Qt::ElideMiddle, 720-64);
		painter->setFont(font);
		painter->drawText(option.rect.x()+48, option.rect.y()+36,
						  option.rect.width()-48, option.rect.height()-32,
						  Qt::AlignTop|Qt::AlignLeft,
						  elided, nullptr);
	}
	else
	{
		// Draw info
		elided = QFontMetrics(font).elidedText(index.data(Qt::UserRole+3).toString(), Qt::ElideMiddle, 720-64);
		painter->setFont(font);
		painter->drawText(option.rect.x()+48, option.rect.y()+36,
						  option.rect.width()-48, option.rect.height()-32,
						  Qt::AlignTop|Qt::AlignLeft,
						  elided, nullptr);
	}

	// Draw icon
	QString info = index.data(Qt::UserRole).toString();
	painter->drawPixmap(option.rect.x(),
						option.rect.y(),
						QIcon::fromTheme(QMimeDatabase().mimeTypeForFile(info).iconName()).pixmap(48,48));

	// Draw name
	font.setPixelSize(32);
	elided = QFontMetrics(font).elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, 720-64);
	painter->setFont(font);
	painter->drawText(option.rect.x()+48, option.rect.y(),
					  option.rect.width()-48, 48,
					  Qt::AlignTop|Qt::AlignLeft,
					  elided, nullptr);
}


