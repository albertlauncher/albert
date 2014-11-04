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
#include "globals.h"
#include "math.h"

#include <QDebug>

/**************************************************************************/
ProposalListView::ProposalListView(QWidget *parent) :
	QListView(parent)
{
	_defaultDelegate = new ProposalListDelegate(Qt::NoModifier);
	_selectedDelegate = new ProposalListDelegate(Qt::NoModifier);
	setItemDelegate(_defaultDelegate);
	setUniformItemSizes(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/**************************************************************************/
ProposalListView::~ProposalListView()
{
	delete _selectedDelegate;
}

/**************************************************************************/
bool ProposalListView::eventFilter(QObject*, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		int key = keyEvent->key();
		Qt::KeyboardModifiers mods = keyEvent->modifiers();

		// Mutual exclusive modifiers
		if ( currentIndex().isValid() && (key == Qt::Key_Control || key == Qt::Key_Meta || key == Qt::Key_Alt)){
			ProposalListDelegate *old = _selectedDelegate;
			if (mods == Qt::ControlModifier || mods == Qt::MetaModifier || mods == Qt::AltModifier)
				_selectedDelegate = new ProposalListDelegate(mods);
			else // there are multiple mod pressed fallback to nomod
				_selectedDelegate = new ProposalListDelegate(Qt::NoModifier);
			setItemDelegateForRow(currentIndex().row(), _selectedDelegate);
			old->deleteLater();
			return true;
		}

		// Navigation
		if (key == Qt::Key_Up || key == Qt::Key_Down
			|| key == Qt::Key_PageDown || key == Qt::Key_PageUp) {
			QListView::keyPressEvent(keyEvent);
			return true;
		}

		// Selection
		if (key == Qt::Key_Return || key == Qt::Key_Enter) {
			if (currentIndex().isValid()) {
				if (mods == Qt::ControlModifier )
					model()->data(currentIndex(), Qt::UserRole+5);
				else if (mods == Qt::MetaModifier)
					model()->data(currentIndex(), Qt::UserRole+6);
				else if (mods == Qt::AltModifier)
					model()->data(currentIndex(), Qt::UserRole+7);
				else //	if (mods == Qt::NoModifier )
					model()->data(currentIndex(), Qt::UserRole+4);
			}
			window()->hide();
			return true;
		}

		// Completion
		if (key == Qt::Key_Tab) {
			if (currentIndex().isValid())
				emit completion(model()->data(currentIndex(), Qt::UserRole+8).toString());
			return true;
		}
	}

	if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		int key = keyEvent->key();
		Qt::KeyboardModifiers mod = keyEvent->modifiers();

		// Modifiers
		if ( currentIndex().isValid() && ( key == Qt::Key_Control || key == Qt::Key_Meta || key == Qt::Key_Alt )){
			ProposalListDelegate *old = _selectedDelegate;
			if ( mod == Qt::ControlModifier || mod == Qt::MetaModifier || mod == Qt::AltModifier)
				_selectedDelegate = new ProposalListDelegate(mod);
			else // there are multiple or none mods pressed fallback to nomod
				_selectedDelegate = new ProposalListDelegate(Qt::NoModifier);
			setItemDelegateForRow(currentIndex().row(), _selectedDelegate);
			old->deleteLater();
			return true;
		}
	}
	return false;
}

/**************************************************************************/
void ProposalListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	QAbstractItemDelegate *a = itemDelegate(current);
	setItemDelegateForRow(current.row(), itemDelegate(previous));
	setItemDelegateForRow(previous.row(), a);
	QListView::currentChanged(current, previous);
}

/**************************************************************************/
QSize ProposalListView::sizeHint() const
{
	if (model()->rowCount() == 0) return QSize(width(), 0);
	int nToShow = std::min(gSettings->value("nItemsToShow", 5).toInt(),
						   model()->rowCount());
	return QSize(width(), nToShow*sizeHintForRow(0));
}

/**************************************************************************/
void ProposalListView::reset()
{
	QAbstractItemView::reset();
	updateGeometry();
	(model()->rowCount() > 0) ? show() : hide();
}
