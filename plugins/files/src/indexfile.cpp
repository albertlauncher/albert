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
#include "indexfile.h"
#include "indextreenode.h"
using namespace std;



/** ***************************************************************************/
Files::IndexFile::IndexFile(QString name, const shared_ptr<Files::IndexTreeNode> &pathNode, QMimeType mimetype)
    : name_(name), pathNode_(pathNode), mimetype_(mimetype){
    name_.squeeze();
}


/** ***************************************************************************/
QString Files::IndexFile::name() const {
    return name_;
}


/** ***************************************************************************/
QString Files::IndexFile::path() const {
    return pathNode_->path();
}


/** ***************************************************************************/
QString Files::IndexFile::filePath() const {
    return QDir(pathNode_->path()).filePath(name_);
}

/** ***************************************************************************/
const QMimeType &Files::IndexFile::mimetype() const {
    return mimetype_;
}
