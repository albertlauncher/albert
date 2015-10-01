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

#include "bookmark.h"
#include "albertapp.h"

QIcon ChromeBookmarks::Bookmark::icon_;


/** ***************************************************************************/
QString ChromeBookmarks::Bookmark::name() const {
    return name_;
}



/** ***************************************************************************/
QString ChromeBookmarks::Bookmark::info() const {
    return url_;
}



/** ***************************************************************************/
QIcon ChromeBookmarks::Bookmark::icon() const {
    return icon_;
}



/** ***************************************************************************/
void ChromeBookmarks::Bookmark::activate() {
    qApp->hideWidget();
    UrlAction(url_).activate();
    ++usage_;
}



/** ***************************************************************************/
vector<QString> ChromeBookmarks::Bookmark::aliases() const {
    return std::vector<QString>({name_});
}
