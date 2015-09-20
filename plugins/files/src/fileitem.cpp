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
#include <QList>
#include <QIcon>
#include "actions/openfileaction.h"
#include "actions/revealfileaction.h"
#include "actions/copyfileaction.h"
#include "actions/copypathaction.h"
#include "fileitem.h"
#include "file.h"
#include "interfaces/iitem.h"

namespace Files {

QHash<QString, QIcon> FileItem::_iconCache;

/** ***************************************************************************/
FileItem::FileItem(File *file, IExtension *ext, IQuery *qry)
    : _file(file), _extension(ext), _query(qry), _actions(nullptr) {}



/** ***************************************************************************/
FileItem::~FileItem() {}



/** ***************************************************************************/
QVariant FileItem::data(int role) const {
    switch (role) {
    case Qt::DisplayRole:
        return QFileInfo(_file->path).fileName();
    case Qt::ToolTipRole:
        return _file->path;
    case Qt:: DecorationRole:
        if (!_iconCache.contains(_file->mimetype.iconName())){
            if (QIcon::hasThemeIcon(_file->mimetype.iconName()))
                _iconCache.insert(_file->mimetype.iconName(),
                                  QIcon::fromTheme(_file->mimetype.iconName()));
            else if(QIcon::hasThemeIcon(_file->mimetype.genericIconName()))
                _iconCache.insert(_file->mimetype.iconName(),
                                  QIcon::fromTheme(_file->mimetype.genericIconName()));
            else
                _iconCache.insert(_file->mimetype.iconName(),
                                  QIcon::fromTheme("unknown"));
        }
        return _iconCache[_file->mimetype.iconName()];
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
void FileItem::activate() {
//    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers(); // TODO:ActionMap

    // Standard action for a file
    OpenFileAction(_file).activate();
}



/** ***************************************************************************/
unsigned short FileItem::score() const {
    return _file->usage;
}



/** ***************************************************************************/
QList<IItem *> FileItem::children() {
    // Lazy instaciate actions
    // NO OWNERSHIP
    return QList<IItem*>({new OpenFileAction(_file),
                          new RevealFileAction(_file),
                          new CopyFileAction(_file),
                          new CopyPathAction(_file)});
}



/** ***************************************************************************/
bool FileItem::hasChildren() const {
    return true;
}



/** ***************************************************************************/
void FileItem::clearIconCache() {
    _iconCache.clear();
}

}
