// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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

#include <QApplication>
#include <QIcon>
#include <QFileInfo>
#include <QDataStream>
#include <QMimeDatabase>
#include "file.h"
#include "fileactions.h"
#include "xdgiconlookup.h"

std::map<QString, Files::File::CacheEntry> Files::File::iconCache_;

/** ***************************************************************************/
QString Files::File::text(const QString &) const {
    return QFileInfo(path_).fileName();
}



/** ***************************************************************************/
QString Files::File::subtext(const QString &) const {
    return path_;
}



/** ***************************************************************************/
QString Files::File::iconPath() const {

    const QString xdgIconName = mimetype_.iconName();
    CacheEntry ce;

    // First check if icon, not older than 15 minutes, exists
    if (iconCache_.count(xdgIconName)){
       ce = iconCache_[xdgIconName];
       if ((std::chrono::system_clock::now() - std::chrono::minutes(15)) < ce.ctime)
           return ce.path;
    }

    QString iconPath;
    QString themeName = QIcon::themeName();
    if ( !(iconPath = XdgIconLookup::instance()->themeIconPath(xdgIconName,themeName)).isNull()  // Lookup iconName
         || !(iconPath = XdgIconLookup::instance()->themeIconPath(mimetype_.genericIconName(),themeName)).isNull()  // Lookup genericIconName
         || !(iconPath = XdgIconLookup::instance()->themeIconPath("unknown",themeName)).isNull()) {  // Lookup "unknown"
        ce = {iconPath, std::chrono::system_clock::now()};
        iconCache_.emplace(xdgIconName, ce);
        return iconPath;
    }

    // Wow nothing found, return empty path
    return QString();
}



/** ***************************************************************************/
void Files::File::activate(ExecutionFlags *flags, const QString &q) {
//    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers(); // TODO:ActionMap
    // Standard action for a file
    OpenFileAction(this).activate(flags, q);
}


/** ***************************************************************************/
vector<SharedAction> Files::File::actions() {
    vector<SharedAction> actions;
    actions.push_back(std::make_shared<RevealFileAction>(this));
    actions.push_back(std::make_shared<CopyFileAction>(this));
    actions.push_back(std::make_shared<CopyPathAction>(this));
    return actions;
}



/** ***************************************************************************/
std::vector<QString> Files::File::indexKeywords() const {
    return std::vector<QString>({ QFileInfo(path_).fileName() });
}



/** ***************************************************************************/
void Files::File::serialize(QDataStream &out) {
    out << path_
        << static_cast<quint16>(usage_)
        << mimetype_.name();
}



/** ***************************************************************************/
void Files::File::deserialize(QDataStream &in) {
    QMimeDatabase db;
    QString mimetype;
    in >> path_ >> usage_ >> mimetype;
    mimetype_ = db.mimeTypeForName(mimetype);
}
