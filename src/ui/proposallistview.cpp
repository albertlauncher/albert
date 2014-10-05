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

#include "proposallistview.h"
#include "proposallistdelegate.h"
#include <QDebug>

#include <QDebug>

/**************************************************************************//**
 * @brief ProposalListView::ProposalListView
 * @param parent
 */
ProposalListView::ProposalListView(QWidget *parent) :
	QListView(parent)
{
	_nItemsToShow = 5;
	setItemDelegate(new ProposalListDelegate);
	setObjectName(QString::fromLocal8Bit("ProposalListWidget"));
	setUniformItemSizes(true);
	setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
//	setLayoutMode(QListView::Batched);
//	setBatchSize(20);
}

/**************************************************************************//**
 * @brief ProposalListView::eventFilter
 * @param event
 * @return
 */
bool ProposalListView::eventFilter(QObject*, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

		// Modifiers
		Qt::KeyboardModifiers mods = _mods ^ keyEvent->modifiers();
		if (mods & Qt::ControlModifier){
			update(currentIndex());
			_mods = keyEvent->modifiers();
			return true;
		}
		if (mods & Qt::AltModifier){
			update(currentIndex());
			_mods = keyEvent->modifiers();
			return true;
		}

		// Navigation
		if (keyEvent->key() == Qt::Key_Up
			|| keyEvent->key() == Qt::Key_Down
			|| keyEvent->key() == Qt::Key_PageDown
			|| keyEvent->key() == Qt::Key_PageUp)
		{
			this->keyPressEvent(keyEvent);
			return true;
		}
	}

	if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

		// Modifiers
		Qt::KeyboardModifiers mods = _mods ^ keyEvent->modifiers();
		if (mods & Qt::ControlModifier){
			update(currentIndex());
			_mods = keyEvent->modifiers();
			return true;
		}
		if (mods & Qt::AltModifier){
			update(currentIndex());
			_mods = keyEvent->modifiers();
			return true;
		}
	}
	return false;
}

QSize ProposalListView::sizeHint() const
{
	if (model()->rowCount() == 0)
		return QSize(width(), 0);
	int nToShow = _nItemsToShow < model()->rowCount() ? _nItemsToShow : model()->rowCount();
	return QSize(width(), nToShow*sizeHintForRow(0));
}

void ProposalListView::reset()
{
	setCurrentIndex(model()->index(0, 0));
	QListView::reset();
	updateGeometry();
}
