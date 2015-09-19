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
    QStringList   aliases() const override {
        return QStringList() << QFileInfo(_path).fileName();
    }

    QString _path;
    QMimeType _mimetype;
    mutable unsigned short _usage;
};

}
