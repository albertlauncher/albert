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
#include <QString>
#include <QMimeType>
#include <map>
#include <vector>
#include "core/indexable.h"

namespace Files {

class File : public Core::IndexableItem
{
public:

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString completionString() const override;
    QString iconPath() const override;
    std::vector<Core::IndexableItem::IndexString> indexStrings() const override;
    std::vector<std::shared_ptr<Core::Action>> actions() override;

    /** Return the filename of the file */
    virtual QString name() const = 0;

    /** Return the path exclusive the filename of the file */
    virtual QString path() const = 0;

    /** Return the path inclusive the filename of the file */
    virtual QString filePath() const = 0;

    /** Return the mimetype of the file */
    virtual const QMimeType &mimetype() const = 0;

private:

    static std::map<QString,QString> iconCache_;

};

}
