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
#include "file.h"

namespace Files {

QHash<QString, QIcon> File::_iconCache;

/** ***************************************************************************/
File::File(const QString &path, QMimeType mimetype){
    _path = path.toStdString();
    _path.shrink_to_fit();
    _mimetype = mimetype;
    _usage = 0;
}



/** ***************************************************************************/
QString File::name(){
    return QFileInfo(QString::fromStdString(_path)).fileName();
}



/** ***************************************************************************/
QString File::description(){
    return _mimetype.comment();
}



/** ***************************************************************************/
QStringList File::alises(){
    return QStringList();
}



/** ***************************************************************************/
QIcon File::icon(){
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
uint File::usage(){
    return _usage;
}



/** ***************************************************************************/
QList<std::shared_ptr<Action>> File::actions(){
    return { std::make_shared<OpenFileAction>(*this), std::make_shared<RevealFileAction>(*this) };
}



/** ***************************************************************************/
QString File::path()
{
    return QFileInfo(QString::fromStdString(_path)).path();
}



/** ***************************************************************************/
QString File::absolutePath(){
    return QString::fromStdString(_path);
}



/** ***************************************************************************/
QMimeType File::mimetype(){
    return _mimetype;
}



/** ***************************************************************************/
bool File::isDir(){
    return QFileInfo(QString::fromStdString(_path)).isDir();
}



/** ***************************************************************************/
void File::clearIconCache(){
    _iconCache.clear();
}
}
