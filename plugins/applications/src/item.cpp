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

#include "item.h"
#include "app.h"
#include "actions/launchappaction.h"





/** ***************************************************************************/
Applications::Item::Item(App *app, IExtension *ext, IQuery *qry)
    : _app(app), _extension(ext), _query(qry) {

}



/** ***************************************************************************/
Applications::Item::~Item() {

}



/** ***************************************************************************/
QVariant Applications::Item::data(int role) const {
    switch (role) {
    case Qt::DisplayRole:    return _app->name;
    case Qt::ToolTipRole:    return _app->altName;
    case Qt::DecorationRole: return _app->icon;
    default: return QVariant();
    }
}



/** ***************************************************************************/
void Applications::Item::activate() {
    // Standard action
    LaunchAppAction(_app).activate();
}



/** ***************************************************************************/
unsigned short Applications::Item::score() const {
    return _app->usage;
}



/** ***************************************************************************/
QList<IItem *> Applications::Item::children() {
    // Lazy instaciate actions
    // NO OWNERSHIP
    return QList<IItem*>({new LaunchAppAction(_app)});
}



/** ***************************************************************************/
bool Applications::Item::hasChildren() const {
    return true;
}
