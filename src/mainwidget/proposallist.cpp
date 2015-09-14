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

#include "proposallist.h"
#include <QDebug>

/** ***************************************************************************/
ProposalList::ProposalList(QWidget *parent) : QListView(parent) {
    QSettings s;
    _itemDelegate = new ItemDelegate;
    _itemDelegate->showInfo = s.value(CFG_SHOW_INFO, CFG_SHOW_INFO_DEF).toBool();
    _itemDelegate->showAction = s.value(CFG_SHOW_ACTION, CFG_SHOW_ACTION_DEF).toBool();
    _maxItems  = s.value(CFG_MAX_PROPOSALS, CFG_MAX_PROPOSALS_DEF).toUInt();

    setItemDelegate(_itemDelegate);
    setUniformItemSizes(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}



/** ***************************************************************************/
ProposalList::~ProposalList() {
    QSettings s;
    s.setValue(CFG_SHOW_INFO, _itemDelegate->showInfo);
    s.setValue(CFG_SHOW_ACTION, _itemDelegate->showAction);
    s.setValue(CFG_MAX_PROPOSALS, _maxItems);
}



/** ***************************************************************************/
bool ProposalList::eventFilter(QObject*, QEvent *event)
{
    if (model() == nullptr)
        return false;

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        // Mods changed -> refresh
        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta){
            update(currentIndex());
            return true;
        }

        // Navigation
        if (key == Qt::Key_Down
                || (key == Qt::Key_Up && currentIndex().isValid()) /* No current -> command history */
                || key == Qt::Key_PageDown
                || key == Qt::Key_PageUp) {
            keyPressEvent(keyEvent);
            return true;
        }

        // Selection
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            // Ignore empty results
            if (model()->rowCount() == 0)
                return true;

            // Select first if none is selected
            if (!currentIndex().isValid())
                setCurrentIndex(model()->index(0,0));

            keyPressEvent(keyEvent); // emits activated
            // Do not accept since the inpuline needs
            //to store the request in history
            window()->hide(); // TODO: Decision of extensions
            return false;
        }

        // Show actions
        if (key == Qt::Key_Tab){
            if (!rootIndex().isValid()){
                if (currentIndex().isValid())
                    setRootIndex(currentIndex());
            }
            else
                setRootIndex(QModelIndex());
            return true;
        }
    }

    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        // Display different subtexts according to the KeyboardModifiers
        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta){
            update(currentIndex());
            return true;
        }
    }
    return false;
}



/** ***************************************************************************/
QSize ProposalList::sizeHint() const
{
    if (model() == nullptr) return QSize();
    uint curr = model()->rowCount();
    int nToShow = _maxItems<curr?_maxItems:curr;
	return QSize(width(), nToShow*sizeHintForRow(0));
}



/** ***************************************************************************/
void ProposalList::reset()
{
    if (model() == nullptr) return;

    // Reset the views state
    QListView::reset();

    // Make the size of this widget be adjusted (size hint changed)
    updateGeometry();

    // Show if not empty and make first item current
    if ( model()->rowCount() > 0 ){
        setCurrentIndex(model()->index(0,0));
        show();
    }
    else
        hide();
}



/** ***************************************************************************/
void ProposalList::setShowInfo(bool b) {
    _itemDelegate->showInfo=b;
}



/** ***************************************************************************/
void ProposalList::setShowAction(bool b) {
    _itemDelegate->showAction=b;
}



/** ***************************************************************************/
void ProposalList::setMaxItems(uint maxItems) {
    _maxItems = maxItems;
}



/** ***************************************************************************/
bool ProposalList::showInfo() const {
    return _itemDelegate->showInfo;
}



/** ***************************************************************************/
bool ProposalList::showAction() const {
    return _itemDelegate->showAction;
}



/** ***************************************************************************/
bool ProposalList::maxItems() const {
    return _maxItems;
}
