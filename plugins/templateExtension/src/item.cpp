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
Template::Item::Item(){

}



/** ***************************************************************************/
Template::Item::~Item(){

}



/** ***************************************************************************/
QString Template::Item::name() const {
    return "Title of item";
}



/** ***************************************************************************/
QString Template::Item::info() const {
    return "Info about the item";
}



/** ***************************************************************************/
QIcon Template::Item::icon() const {
    // Icon of item
    return QIcon();
}



/** ***************************************************************************/
void Template::Item::activate() {
    // Do sth cool...
}



/** ***************************************************************************/
bool Template::Item::hasChildren() const {
    // Performance measure.
    return false;
}



/** ***************************************************************************/
vector<shared_ptr<A2Item>> Template::Item::children() {
    // Return the children.
    // Did not want to have children? Subclass A2leaf instead.
    return vector<shared_ptr<A2Item>>();
}

