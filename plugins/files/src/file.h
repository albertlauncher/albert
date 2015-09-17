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
#include <QIcon>
#include <QMimeType>
#include <string>
#include "interfaces.h"
#include "utils/search/iindexable.h"


namespace Files {

class Extension;

class File final : public RootNode, public IIndexable
{
    friend class ScanWorker;

public:
    explicit File(const QString &path, QMimeType mimetype);

    QString       name(const Query *) const override;
    QString       description(const Query *) const override;
    QIcon         icon() const override;
    void          activate(const Query *) override;
    uint          usage() const override;
    QList<INode*> children() override;
    QStringList   aliases() const override;

    QString path() const;
    QString absolutePath() const;
    QMimeType mimetype() const;
    bool isDir() const;
    static void clearIconCache();

private:
    QString _path;
    QMimeType _mimetype;
    uint _usage;
    QList<INode*> *_actions;
    static QHash<QString, QIcon> _iconCache;
};
}
