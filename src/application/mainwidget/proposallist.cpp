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
    _itemDelegate->showInfo   = s.value(CFG_SHOW_INFO, CFG_SHOW_INFO_DEF).toBool();
    _itemDelegate->showForSelectedOnly = s.value(CFG_SELECTED_ONLY, CFG_SELECTED_ONLY_DEF).toBool();
    _maxItems  = s.value(CFG_MAX_PROPOSALS, CFG_MAX_PROPOSALS_DEF).toInt();

    setItemDelegate(_itemDelegate);
    setUniformItemSizes(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}



/** ***************************************************************************/
ProposalList::~ProposalList() {
    QSettings s;
    s.setValue(CFG_SHOW_INFO, _itemDelegate->showInfo);
    s.setValue(CFG_SELECTED_ONLY, _itemDelegate->showForSelectedOnly);
    s.setValue(CFG_MAX_PROPOSALS, _maxItems);
    delete _itemDelegate;
}



/** ***************************************************************************/
bool ProposalList::eventFilter(QObject*, QEvent *event) {
    if (model() == nullptr)
        return false;

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        // Mods changed -> refresh
        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
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
            if (!model()->hasChildren(rootIndex()))
                return true;

            // Select first if none is selected
            if (!currentIndex().isValid())
                setCurrentIndex(model()->index(0, 0, rootIndex()));

            keyPressEvent(keyEvent); // emits activated
            // Do not accept since the inpuline needs
            //to store the request in history
            return false;
        }

        // Show actions
        if (key == Qt::Key_Tab) {

            // Skip if view is empty
            if (!model()->hasChildren(rootIndex()))
                return true;

            // If none is selected use the first
            QModelIndex midx;
            if (currentIndex().isValid())
                midx = currentIndex();
            else
                midx = model()->index(0, 0, rootIndex());

            // If view is in a subtree...
            if (rootIndex().isValid()){
                // Change to Toplevel
                setRootIndex(QModelIndex());
                setCurrentIndex(midx.parent());
            } else {
                // Change to children if there are any
                if (model()->hasChildren(midx)){
                    setRootIndex(midx);
                    setCurrentIndex(QModelIndex());
                }
            }

            updateGeometry();
            return true;
        }
    }

    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        // Display different subtexts according to the KeyboardModifiers
        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
            update(currentIndex());
            return true;
        }
    }
    return false;
}



/** ***************************************************************************/
QSize ProposalList::sizeHint() const {
    if (model() == nullptr)
        return QSize();
    int cnt = model()->rowCount(rootIndex());
    int nToShow = (_maxItems < cnt) ? _maxItems : cnt;
    return QSize(width(), nToShow*sizeHintForRow(0));
}



/** ***************************************************************************/
void ProposalList::reset() {
    // Reset the views state
    QListView::reset();
    scrollToTop(); // Why is this needed?

    // Show if not empty and make first item current
    if (model()!=nullptr && model()->hasChildren(rootIndex())) {
        show();
        // Make the size of this widget be adjusted (size hint changed)
        updateGeometry();
    }
    else
        hide();
}



/** ***************************************************************************/
void ProposalList::setShowInfo(bool b) {
    _itemDelegate->showInfo=b;
}



/** ***************************************************************************/
bool ProposalList::showInfo() const {
    return _itemDelegate->showInfo;
}



/** ***************************************************************************/
void ProposalList::setSelectedOnly(bool b) {
    _itemDelegate->showForSelectedOnly=b;
}



/** ***************************************************************************/
bool ProposalList::selectedOnly() const {
    return _itemDelegate->showForSelectedOnly;
}



/** ***************************************************************************/
void ProposalList::setMaxItems(int maxItems) {
    _maxItems = maxItems;
}



/** ***************************************************************************/
int ProposalList::maxItems() const {
    return _maxItems;
}
