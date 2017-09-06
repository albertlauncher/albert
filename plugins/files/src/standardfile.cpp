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

#include <QDir>
#include <QFileInfo>
#include "standardfile.h"
using namespace std;


/** ***************************************************************************/
Files::StandardFile::StandardFile(QString path, QMimeType mimetype)
    : mimetype_(mimetype){
    QFileInfo fileInfo(path);
    name_ = fileInfo.fileName();
    name_.squeeze();
    path_ = fileInfo.canonicalPath();
    path_.squeeze();
}


/** ***************************************************************************/
QString Files::StandardFile::name() const {
    return name_;
}


/** ***************************************************************************/
QString Files::StandardFile::path() const {
    return path_;
}


/** ***************************************************************************/
QString Files::StandardFile::filePath() const {
    return QDir(path_).filePath(name_);
}

/** ***************************************************************************/
const QMimeType &Files::StandardFile::mimetype() const {
    return mimetype_;
}
