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
FirefoxBookmarks::Item::Item(){

}



/** ***************************************************************************/
FirefoxBookmarks::Item::~Item(){

}



/** ***************************************************************************/
QString FirefoxBookmarks::Item::id() const {
    return "Id of item";
}



/** ***************************************************************************/
QString FirefoxBookmarks::Item::text() const {
    return "Title of item";
}



/** ***************************************************************************/
QString FirefoxBookmarks::Item::subtext() const {
    return "Info about the item";
}



/** ***************************************************************************/
QString FirefoxBookmarks::Item::iconPath() const {
    // Icon of item
    return QString();
}
