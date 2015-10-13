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
#include <QIcon>
#include <vector>
#include <map>
#include <memory>
using std::vector;
using std::map;
using std::shared_ptr;
using std::unique_ptr;
#include "abstractobjects.h"
#include "search/iindexable.h"

namespace Files {

class File final : public ActionNode, public IIndexable
{
    friend class Extension;
    friend class Indexer;

public:
    File() = delete;
    File(const File &) = delete;
    File(QString path, QMimeType mimetype, short usage = 0);

    QString text() const override;
    QString subtext() const override;
    QIcon icon() const override;
    void activate() override;
    bool hasChildren() const override;
    vector<shared_ptr<ActionNode>> children() override;
    vector<QString> aliases() const override;

    const QString &path() const { return path_; }
    const QMimeType &mimetype() const { return mimetype_; }
    short usage() const { return usage_; }
    void incUsage() {++usage_;}

    static void clearIconCache();

private:
    QString path_;
    QMimeType mimetype_;
    mutable short usage_;
    unique_ptr<vector<shared_ptr<ActionNode>>> children_;
    static map<QString, QIcon> iconCache_;
};

}
