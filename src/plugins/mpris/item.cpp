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


/** ***************************************************************************/
MPRIS::Item::Item(){

}



/** ***************************************************************************/
MPRIS::Item::~Item(){

}



/** ***************************************************************************/
QString MPRIS::Item::text() const {
    return "Title of item";
}



/** ***************************************************************************/
QString MPRIS::Item::subtext() const {
    return "Info about the item";
}



/** ***************************************************************************/
QString MPRIS::Item::iconPath() const {
    // Icon of item
    return QString();
}



/** ***************************************************************************/
void MPRIS::Item::activate() {
    // Do sth cool...
}



/** ***************************************************************************/
bool MPRIS::Item::hasChildren() const {
    // Performance measure.
    return false;
}



/** ***************************************************************************/
vector<shared_ptr<AlbertItem>> MPRIS::Item::children() {
    // Return the children.
    // Did not want to have children? Subclass A2leaf instead.
    return vector<shared_ptr<AlbertItem>>();
}

