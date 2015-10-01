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
#include "file.h"
#include "fileactions.h"


map<QString, QIcon> Files::File::iconCache_;


/** ***************************************************************************/
Files::File::File(QString path, QMimeType mimetype, short usage)
    : path_(path), mimetype_(mimetype), usage_(usage){}



/** ***************************************************************************/
QString Files::File::name() const {
    return QFileInfo(path_).fileName();
}



/** ***************************************************************************/
QString Files::File::info() const {
    return path_;
}



/** ***************************************************************************/
QIcon Files::File::icon() const {
    QString iconName = mimetype_.iconName();
    if (iconCache_.count(iconName) == 0) {
        if (QIcon::hasThemeIcon(iconName))
            iconCache_[iconName] = QIcon::fromTheme(iconName);
        else if(QIcon::hasThemeIcon(mimetype_.genericIconName()))
            iconCache_[iconName] = QIcon::fromTheme(mimetype_.genericIconName());
        else
            iconCache_[iconName] = QIcon::fromTheme("unknown");
    }
    return iconCache_[iconName];
}



/** ***************************************************************************/
void Files::File::activate() {
//    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers(); // TODO:ActionMap
    // Standard action for a file
    OpenFileAction(this).activate();
}


/** ***************************************************************************/
bool Files::File::hasChildren() const {
    return true;
}



/** ***************************************************************************/
vector<shared_ptr<A2Item>> Files::File::children() {
    // Lazy instaciate actions
    if (!children_){
        children_ = unique_ptr<vector<shared_ptr<A2Item>>>(new vector<shared_ptr<A2Item>>);
        children_->push_back(std::make_shared<OpenFileAction>(this));
        children_->push_back(std::make_shared<RevealFileAction>(this));
        children_->push_back(std::make_shared<CopyFileAction>(this));
        children_->push_back(std::make_shared<CopyPathAction>(this));
    }
    return *children_;
}



/** ***************************************************************************/
std::vector<QString> Files::File::aliases() const {
    return std::vector<QString>({QFileInfo(path_).fileName()});
}



/** ***************************************************************************/
void Files::File::clearIconCache() {
    iconCache_.clear();
}

