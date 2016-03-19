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

#include <QApplication>
#include <QFileInfo>
#include "file.h"
#include "fileactions.h"
#include "iconlookup/xdgiconlookup.h"


/** ***************************************************************************/
QString Files::File::text() const {
    return QFileInfo(path_).fileName();
}



/** ***************************************************************************/
QString Files::File::subtext() const {
    return path_;
}



/** ***************************************************************************/
QString Files::File::iconPath() const {
    XdgIconLookup xdg;

    QString iconPath = xdg.themeIcon(mimetype_.iconName());
    if (!iconPath.isNull())
        return iconPath;

    iconPath = xdg.themeIcon(mimetype_.genericIconName());
    if (!iconPath.isNull())
        return iconPath;

    iconPath = xdg.themeIcon("unknown");
    if (!iconPath.isNull())
        return iconPath;

    return QString();

//    // First check cache
//    QString xdgIconName = mimetype_.iconName();
//    if (iconCache_.count(xdgIconName))
//        return iconCache_[xdgIconName];

//    // Lookup iconName
//    XdgIconLookup xdg;
//    QString iconPath;
//    iconPath = xdg.lookupIcon(xdgIconName);
//    if (!iconPath.isNull()){
//        iconCache_.emplace(xdgIconName, QUrl::fromLocalFile(iconPath));
//        return QUrl::fromLocalFile(iconPath);
//    }

//    // Lookup genericIconName
//    QString genericIconName = mimetype_.genericIconName();
//    iconPath = xdg.lookupIcon(genericIconName);
//    if (!iconPath.isNull()){
//        iconCache_.emplace(xdgIconName, QUrl::fromLocalFile(iconPath)); // Intended
//        return QUrl::fromLocalFile(iconPath);
//    }

//    // Lookup "unknown"
//    iconPath = xdg.lookupIcon("unknown");
//    if (!iconPath.isNull()){
//        iconCache_.emplace(xdgIconName, QUrl::fromLocalFile(iconPath)); // Intended
//        return QUrl::fromLocalFile(iconPath);
//    }

//    // Return empty url
//    return QUrl();
}



/** ***************************************************************************/
void Files::File::activate() {
//    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers(); // TODO:ActionMap
    // Standard action for a file
    OpenFileAction(this).activate();
}


/** ***************************************************************************/
ActionSPtrVec Files::File::actions() {
    ActionSPtrVec actions;
    actions.push_back(std::make_shared<RevealFileAction>(this));
    actions.push_back(std::make_shared<CopyFileAction>(this));
    actions.push_back(std::make_shared<CopyPathAction>(this));
    return actions;
}



/** ***************************************************************************/
std::vector<QString> Files::File::aliases() const {
    return std::vector<QString>({QFileInfo(path_).fileName()});
}
