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

#pragma once
#include <QString>
#include <QMimeType>
#include <QFileInfo>
#include "utils/search/iindexable.h"

namespace Files {

struct File final : public IIndexable
{
    File() {}
    File(QString path, QMimeType mimetype)
        : path(path), mimetype(mimetype), usage(0){}
    File(const File &other)
        : path(other.path), mimetype(other.mimetype), usage(other.usage){}
    ~File(){}


    QStringList   aliases() const override {
        return QStringList() << QFileInfo(path).fileName();
    }

    QString path;
    QMimeType mimetype;
    mutable unsigned short usage;
};

}
