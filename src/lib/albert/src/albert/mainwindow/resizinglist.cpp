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

#include "resizinglist.h"
#include <QDebug>

/** ***************************************************************************/
uint8_t ResizingList::maxItems() const {
    return maxItems_;
}



/** ***************************************************************************/
void ResizingList::setMaxItems(uint8_t maxItems) {
    maxItems_ = maxItems;
    updateGeometry();
}



/** ***************************************************************************/
QSize ResizingList::sizeHint() const {
    if (model() == nullptr)
        return QSize();
    return QSize(width(), sizeHintForRow(0) * std::min(static_cast<int>(maxItems_), model()->rowCount(rootIndex())));
}



/** ***************************************************************************/
QSize ResizingList::minimumSizeHint() const {
    return QSize(0,0); // Fix for small lists
}



/** ***************************************************************************/
void ResizingList::setModel(QAbstractItemModel * m) {
    if (model() == m)
        return;

    if (model()!=nullptr) {
        disconnect(this->model(), &QAbstractItemModel::rowsInserted, this, &ResizingList::updateAppearance);
        disconnect(this->model(), &QAbstractItemModel::modelReset, this, &ResizingList::updateAppearance);
    }

    QItemSelectionModel *sm = selectionModel();
    QAbstractItemView::setModel(m);
    delete sm;
    updateAppearance();

    // If not empty show and select first, update geom. If not null connect.
    if (model()!=nullptr) {
        connect(this->model(), &QAbstractItemModel::rowsInserted, this, &ResizingList::updateAppearance);
        connect(this->model(), &QAbstractItemModel::modelReset, this, &ResizingList::updateAppearance);
    }
}



/** ***************************************************************************/
void ResizingList::updateAppearance() {
    if ( model() == nullptr || model()->rowCount() == 0 )
        hide();
    else {
        show();
        if ( !currentIndex().isValid() )
            setCurrentIndex(model()->index(0, 0));
        updateGeometry();
    }
}

