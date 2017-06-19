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

#pragma once
#include <QMimeType>
#include <map>
#include <memory>
#include <vector>
#include "core/indexable.h"
#include "core/item.h"

namespace Files {

class File final : public Core::Item, public Core::Indexable
{
public:

    File() {}
    File(QString path, QMimeType mimetype)
        : path_(path), mimetype_(mimetype){}

    /*
     * Implementation of Item interface
     */

    QString id() const override { return path_; }
    QString text() const override;
    QString subtext() const override;
    QString completionString() const override;
    QString iconPath() const override;
    std::vector<Core::Indexable::WeightedKeyword> indexKeywords() const override;
    std::vector<std::shared_ptr<Core::Action>> actions() override;

    /*
     * Item specific members
     */

    /** Return the path of the file */
    const QString &path() const { return path_; }

    /** Return the mimetype of the file */
    const QMimeType &mimetype() const { return mimetype_; }

private:

    QString path_;
    QMimeType mimetype_;
    static std::map<QString, QString> iconCache_;
};

}
