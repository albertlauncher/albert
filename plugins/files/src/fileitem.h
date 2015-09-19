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
#include <QVariant>
#include <QList>
#include "interfaces/iitem.h"
class IExtension;
class IQuery;

namespace Files {

class File;

class FileItem final : public IItem
{
public:
    FileItem(File *file, IExtension *ext, IQuery *qry);
    ~FileItem();

    QVariant       data(int role = Qt::DisplayRole) const override;
    void           activate() override;
    unsigned short score() const override;
    QList<IItem*>  children() override;
    bool           hasChildren() const override;
    static void    clearIconCache();

private:
    File          *_file;      // No ownership
    IExtension    *_extension; // No ownership
    IQuery        *_query;     // No ownership
    QList<IItem*> *_actions;   // No ownership
    static QHash<QString, QIcon> _iconCache;
};
}
