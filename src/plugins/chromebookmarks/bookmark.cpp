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

#include <QUrl>
#include <QDesktopServices>
#include <QDataStream>
#include "bookmark.h"
#include "albertapp.h"


/** ***************************************************************************/
QString ChromeBookmarks::Bookmark::text() const {
    return name_;
}



/** ***************************************************************************/
QString ChromeBookmarks::Bookmark::subtext() const {
    return url_;
}



/** ***************************************************************************/
QString ChromeBookmarks::Bookmark::iconPath() const {
    return ":favicon";
}



/** ***************************************************************************/
void ChromeBookmarks::Bookmark::activate() {
    qApp->hideWidget();
    QDesktopServices::openUrl(QUrl(url_));
    ++usage_;
}



/** ***************************************************************************/
vector<QString> ChromeBookmarks::Bookmark::aliases() const {
    // return domain without TLD eg. maps.google for maps.google.de
    QUrl url(url_);
    QString host = url.host();
    return std::vector<QString>({host.left(host.size()-url.topLevelDomain().size())});
}


/** ***************************************************************************/
void ChromeBookmarks::Bookmark::serialize(QDataStream &out) {
    out << name_ << url_ << static_cast<quint16>(usage_);
}



/** ***************************************************************************/
void ChromeBookmarks::Bookmark::deserialize(QDataStream &in) {
    in >> name_ >> url_ >> usage_;
}
