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

#include "file.h"
#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include "xdg/iconlookup.h"
#include "fileactions.h"
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
QString Files::File::completionString() const {
    QString result = ( QFileInfo(path_).isDir() ) ? QString("%1/").arg(path_) : path_;
#ifdef __linux__
    if ( result.startsWith(QDir::homePath()) )
        result.replace(QDir::homePath(), "~");
#endif
    return result;
}



/** ***************************************************************************/
QString Files::File::iconPath() const {

    const QString xdgIconName = mimetype_.iconName();

    // First check if icon exists
    auto search = iconCache_.find(xdgIconName);
    if(search != iconCache_.end())
        return search->second;

    QString icon = XDG::IconLookup::iconPath({xdgIconName, mimetype_.genericIconName(), "unknown"});
    if ( !icon.isEmpty() ) {
        iconCache_.emplace(xdgIconName, icon);
        return icon;
    }

    // Nothing found, return a fallback icon
    if ( xdgIconName == "inode-directory" ) {
        icon = ":directory";
        iconCache_.emplace(xdgIconName, icon);
    } else {
        icon = ":unknown";
        iconCache_.emplace(xdgIconName, icon);
    }
    return icon;
}


/** ***************************************************************************/
vector<shared_ptr<Core::Action>> Files::File::actions() {
    vector<shared_ptr<Core::Action>> actions;
    actions.push_back(std::make_shared<OpenFileAction>(this));
    QFileInfo fileInfo(path_);
    if ( fileInfo.isFile() && fileInfo.isExecutable() )
        actions.push_back(std::make_shared<ExecuteFileAction>(this));
    actions.push_back(std::make_shared<RevealFileAction>(this));
    actions.push_back(std::make_shared<TerminalFileAction>(this));
    actions.push_back(std::make_shared<CopyFileAction>(this));
    actions.push_back(std::make_shared<CopyPathAction>(this));
    return actions;
}



/** ***************************************************************************/
vector<Core::IndexableItem::WeightedKeyword> Files::File::indexKeywords() const {
    std::vector<IndexableItem::WeightedKeyword> res;
    res.emplace_back(QFileInfo(path_).fileName(), UINT_MAX);
    // TODO ADD PATH
    return res;
}
