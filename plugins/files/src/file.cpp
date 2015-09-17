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

#include <QFileInfo>
#include <QApplication>
#include "file.h"
#include "actions/openfileaction.h"
#include "actions/revealfileaction.h"
#include "actions/copyfileaction.h"
#include "actions/copypathaction.h"
#include "extension.h"

namespace Files {

QHash<QString, QIcon> File::_iconCache;

/** ***************************************************************************/
File::File(const QString &path, QMimeType mimetype)
    : _path(path), _mimetype(mimetype), _usage(0), _actions(nullptr) {}



/** ***************************************************************************/
QString File::name(const Query *) const {
    return QFileInfo(_path).fileName();
}



/** ***************************************************************************/
QString File::description(const Query *) const {
    return _path;
}



/** ***************************************************************************/
void File::activate(const Query *q) {
//    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers(); // TODO:ActionMap
    OpenFileAction(this).activate(q);
}



/** ***************************************************************************/
QIcon File::icon() const {
    if (!_iconCache.contains(_mimetype.iconName())){
        if (QIcon::hasThemeIcon(_mimetype.iconName()))
            _iconCache.insert(_mimetype.iconName(), QIcon::fromTheme(_mimetype.iconName()));
        else if(QIcon::hasThemeIcon(_mimetype.genericIconName()))
            _iconCache.insert(_mimetype.iconName(), QIcon::fromTheme(_mimetype.genericIconName()));
        else
            _iconCache.insert(_mimetype.iconName(), QIcon::fromTheme("unknown"));
    }
    return _iconCache[_mimetype.iconName()];
}



/** ***************************************************************************/
QStringList File::aliases() const {
    return QStringList() << QFileInfo(_path).fileName();
}



/** ***************************************************************************/
uint File::usage() const {
    return _usage;
}



/** ***************************************************************************/
QList<INode *> File::children() {
    if (_actions == nullptr){
        _actions = new QList<INode*>({new OpenFileAction(this),
                                     new RevealFileAction(this),
                                     new CopyFileAction(this),
                                     new CopyPathAction(this)});
    }
    return *_actions;
}



/** ***************************************************************************/
QString File::path() const {
    return QFileInfo(_path).path();
}



/** ***************************************************************************/
QString File::absolutePath() const {
    return _path;
}



/** ***************************************************************************/
QMimeType File::mimetype() const {
    return _mimetype;
}



/** ***************************************************************************/
bool File::isDir() const {
    return QFileInfo(_path).isDir();
}



/** ***************************************************************************/
void File::clearIconCache() {
    _iconCache.clear();
}
}
