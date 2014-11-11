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
#include "math.h"

#include <QStyledItemDelegate>
#include <QPainter>

#include <QDebug>

/******************************************************************************/
/************************  B E G I N   P R I V A T E  *************************/
/******************************************************************************/


/**************************************************************************/
class StandardDelegate : public QStyledItemDelegate
{
protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		painter->save();
		QStyle *style = option.widget->style();
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

		QRect contentsRect = style->subElementRect(QStyle::SE_ItemViewItemText,
												   &option,
												   option.widget);
		int a = contentsRect.height();

		/* Draw DecorationRole */
		QRect DecorationRect(contentsRect.topLeft(), option.decorationSize);
		DecorationRect.translate( (a-option.decorationSize.width())/2, (a-option.decorationSize.height())/2);
		painter->drawPixmap(DecorationRect, index.data(Qt::DecorationRole).value<QIcon>().pixmap(option.decorationSize));

		/* Draw DisplayRole */
		QRect DisplayRect(contentsRect.adjusted(a+3,0,0,0));
		QString text = QFontMetrics(option.font).elidedText(
					index.data(Qt::DisplayRole).toString(),
					option.textElideMode,
					DisplayRect.width());
		painter->drawText(DisplayRect, Qt::AlignVCenter|Qt::AlignLeft, text);
		painter->restore();
	}
};

/**************************************************************************/
class SubTextDelegate : public QStyledItemDelegate
{
	int _subtextRole;

public:
	SubTextDelegate(int subtextRole) : _subtextRole(subtextRole){}

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
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
					index.data(_subtextRole).toString(),
					option.textElideMode,
					URTextRect.width());
		painter->setFont(font);
		painter->drawText(URTextRect, Qt::AlignVCenter|Qt::AlignLeft, text);

		painter->restore();
	}
};


/******************************************************************************/
/**************************  E N D   P R I V A T E  ***************************/
/******************************************************************************/


/**************************************************************************/
ProposalListView::ProposalListView(QWidget *parent) :
	QListView(parent)
{
	_selectedDelegate = nullptr;
	_subModeSelIsAction = false;
	_subModeDefIsAction = false;

	/* Apply initial delegates */
	setSubModeSel(SubTextMode::None);
	setSubModeDef(SubTextMode::None);

	// List properties
	setUniformItemSizes(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/**************************************************************************/
ProposalListView::~ProposalListView()
{
	delete _selectedDelegate;
}

/**************************************************************************/
void ProposalListView::modifyDelegate(Qt::KeyboardModifiers mods)
{
	if (_subModeDefIsAction){
		delete itemDelegate();
		switch (mods) {
		case Qt::ControlModifier:
			setItemDelegate(new SubTextDelegate(Qt::UserRole+10+_actionCtrl));
			break;
		case Qt::MetaModifier:
			setItemDelegate(new SubTextDelegate(Qt::UserRole+10+_actionMeta));
			break;
		case Qt::AltModifier:
			setItemDelegate(new SubTextDelegate(Qt::UserRole+10+_actionAlt));
			break;
		default:
			setItemDelegate(new SubTextDelegate(Qt::UserRole+10));
			break;
		}
		update();
	}

	if (_subModeSelIsAction){
		delete _selectedDelegate;
		switch (mods) {
		case Qt::ControlModifier:
			_selectedDelegate = new SubTextDelegate(Qt::UserRole+10+_actionCtrl);
			break;
		case Qt::MetaModifier:
			_selectedDelegate = new SubTextDelegate(Qt::UserRole+10+_actionMeta);
			break;
		case Qt::AltModifier:
			_selectedDelegate = new SubTextDelegate(Qt::UserRole+10+_actionAlt);
			break;
		default:
			_selectedDelegate = new SubTextDelegate(Qt::UserRole+10);
			break;
		}

		// Give all the custom rows the new delegate
		// and let the new delgates be visible
		for (int i : _customDelegateRows){
			setItemDelegateForRow(i, _selectedDelegate);
			update(model()->index(i,0));
		}
	}
}

/**************************************************************************/
void ProposalListView::setSubModeSel(ProposalListView::SubTextMode m)
{
	_subModeSelIsAction = ( m == SubTextMode::Action );

	// Replace the delegate by a new cool one
	delete _selectedDelegate;
	switch (m) {
	case SubTextMode::None:
		_selectedDelegate = new StandardDelegate;
		break;
	case SubTextMode::Info:
		_selectedDelegate = new SubTextDelegate(Qt::ToolTipRole);
		break;
	case SubTextMode::Action:
		_selectedDelegate = new SubTextDelegate(Qt::UserRole+10);
		break;
	}

	// Give all the custom rows the new delegate
	for (int i : _customDelegateRows)
		setItemDelegateForRow(i, _selectedDelegate);

	// Let the new delgates be visible
	update();

}

/**************************************************************************/
void ProposalListView::setSubModeDef(ProposalListView::SubTextMode m)
{
	_subModeDefIsAction = ( m == SubTextMode::Action );

	// Replace the delegate by a new cool one
	delete itemDelegate();
	switch (m) {
	case SubTextMode::None:
		setItemDelegate(new StandardDelegate);
		break;
	case SubTextMode::Info:
		setItemDelegate(new SubTextDelegate(Qt::ToolTipRole));
		break;
	case SubTextMode::Action:
		setItemDelegate(new SubTextDelegate(Qt::UserRole+10));
		break;
	}

	// Let the new delgates be visible
	update();
}

/**************************************************************************/
bool ProposalListView::eventFilter(QObject*, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		int key = keyEvent->key();
		Qt::KeyboardModifiers mods = keyEvent->modifiers();

		// Display different subtexts according to the KeyboardModifiers
		if ( (key == Qt::Key_Control || key == Qt::Key_Meta || key == Qt::Key_Alt)){
			modifyDelegate(mods);
			return true;
		}

		// Navigation
		if (key == Qt::Key_Up || key == Qt::Key_Down
			|| key == Qt::Key_PageDown || key == Qt::Key_PageUp) {

			/* I this is the first item pass key up through for the
			 * command history */
			if (key == Qt::Key_Up && (!currentIndex().isValid() || currentIndex().row()==0))
				return false;

			QListView::keyPressEvent(keyEvent);
			return true;
		}

		// Selection
		if (key == Qt::Key_Return || key == Qt::Key_Enter) {
			if (!currentIndex().isValid()){
				if (model()->rowCount() > 0)
					setCurrentIndex(model()->index(0,0));
				else
					return true;
			}

			if (mods == Qt::ControlModifier )
				model()->data(currentIndex(), Qt::UserRole+20+_actionCtrl);
			else if (mods == Qt::MetaModifier)
				model()->data(currentIndex(), Qt::UserRole+20+_actionMeta);
			else if (mods == Qt::AltModifier)
				model()->data(currentIndex(), Qt::UserRole+20+_actionAlt);
			else //	if (mods == Qt::NoModifier )
				model()->data(currentIndex(), Qt::UserRole+20);

			window()->hide();
			// Do not accept since the inpuline needs
			// to store the request in history
			return false;
		}

		// Completion
		if (key == Qt::Key_Tab) {
			if (!currentIndex().isValid())
				if (model()->rowCount() > 0)
					emit completion(model()->data(model()->index(0,0), Qt::UserRole).toString());
				else
					return true;
			else
				emit completion(model()->data(currentIndex(), Qt::UserRole).toString());
			return true;
		}
	}

	if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		int key = keyEvent->key();

		// Display different subtexts according to the KeyboardModifiers
		if ( (key == Qt::Key_Control || key == Qt::Key_Meta || key == Qt::Key_Alt)){
			modifyDelegate(keyEvent->modifiers());
			return true;
		}
	}
	return false;
}

/**************************************************************************/
void ProposalListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	// Show specialized hints in the current item
	QListView::currentChanged(current, previous);
	if (previous.isValid()){
		setItemDelegateForRow(previous.row(), nullptr);
		_customDelegateRows.remove(previous.row());
	}

	setItemDelegateForRow(current.row(), _selectedDelegate);
	_customDelegateRows.insert(current.row());
}

/**************************************************************************/
QSize ProposalListView::sizeHint() const
{
	if (model()->rowCount() == 0) return QSize(width(), 0);
	int nToShow = std::min(_nItemsToShow, model()->rowCount());
	return QSize(width(), nToShow*sizeHintForRow(0));
}

/**************************************************************************/
void ProposalListView::saveSettings(QSettings &s) const
{
	s.setValue("ActionCtrl", _actionCtrl);
	s.setValue("ActionMeta", _actionMeta);
	s.setValue("ActionAlt", _actionAlt);
	s.setValue("SubtextModeSelection", static_cast<int>(_selSubtextMode));
	s.setValue("SubtextModeDefault", static_cast<int>(_defSubtextMode));
	s.setValue("NumberOfProposals", _nItemsToShow);
}

/**************************************************************************/
void ProposalListView::loadSettings(QSettings &s)
{
	_actionCtrl = s.value("ActionCtrl",1).toInt();
	_actionMeta = s.value("ActionMeta",0).toInt();
	_actionAlt = s.value("ActionAlt",2).toInt();
	_selSubtextMode = static_cast<SubTextMode>(s.value("SubtextModeSelection", 2).toInt());
	_defSubtextMode = static_cast<SubTextMode>(s.value("SubtextModeDefault", 1).toInt());
	_nItemsToShow = s.value("NumberOfProposals", 6).toInt();
}

/**************************************************************************/
void ProposalListView::reset()
{
	/*
	 * Unset all custom delegates. This is not possible in 5.3.3.
	 * Anyway this should be done by QListView::reset(); [ See QTBUG-42391 ]
	 */
	for (int i : _customDelegateRows)
		setItemDelegateForRow(i, nullptr);
	_customDelegateRows.clear();

	// Reset the  views state
	QListView::reset();

	// Make the size of this widget be adjusted (size hint changed)
	updateGeometry();

	// Show if not empty and make first item current
	if ( model()->rowCount() > 0 ){
		show();
		setCurrentIndex(model()->index(0,0));
	}
	else
		hide();
}
