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
#include <QFileInfo>
#include <QDataStream>
#include <QMimeDatabase>
#include "file.h"
#include "fileactions.h"
#include "xdgiconlookup.h"
using std::vector;
using std::shared_ptr;

std::map<QString,QString> Files::File::iconCache_;

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

    const QString xdgIconName = mimetype_.iconName();

    // First check if icon exists
    if (iconCache_.count(xdgIconName))
        return iconCache_[xdgIconName];

    QString iconPath;
    if ( !(iconPath = XdgIconLookup::iconPath(xdgIconName)).isNull()  // Lookup iconName
         || !(iconPath = XdgIconLookup::iconPath(mimetype_.genericIconName())).isNull()  // Lookup genericIconName
         || !(iconPath = XdgIconLookup::iconPath("unknown")).isNull()) {  // Lookup "unknown"
        iconCache_.emplace(xdgIconName, iconPath);
        return iconPath;
    }

    // Nothing found, return a fallback icon
    if ( xdgIconName == "inode-directory" ) {
        iconPath = ":directory";
        iconCache_.emplace(xdgIconName, iconPath);
    } else {
        iconPath = ":unknown";
        iconCache_.emplace(xdgIconName, iconPath);
    }
    return iconPath;
}


/** ***************************************************************************/
vector<shared_ptr<Core::Action>> Files::File::actions() {
    vector<shared_ptr<Core::Action>> actions;
    actions.push_back(std::make_shared<OpenFileAction>(this));
    actions.push_back(std::make_shared<RevealFileAction>(this));
    actions.push_back(std::make_shared<TerminalFileAction>(this));
    actions.push_back(std::make_shared<CopyFileAction>(this));
    actions.push_back(std::make_shared<CopyPathAction>(this));
    return actions;
}



/** ***************************************************************************/
vector<Core::Indexable::WeightedKeyword> Files::File::indexKeywords() const {
    std::vector<Indexable::WeightedKeyword> res;
    res.emplace_back(QFileInfo(path_).fileName(), USHRT_MAX);
    // TODO ADD PATH
    return res;
}
