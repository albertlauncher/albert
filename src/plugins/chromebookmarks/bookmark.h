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
#include "abstractobjects.hpp"

namespace ChromeBookmarks {

class Bookmark final : public AlbertItem
{
    friend class Extension;
    friend class Indexer;

public:
    Bookmark() = delete;
    Bookmark(const Bookmark &) = delete;
    Bookmark(const QString &name, const QString &url, short usage = 0)
        : name_(name), url_(url), usage_(usage) {}

    QString text() const override;
    QString subtext() const override;
    QIcon icon() const override;
    void activate() override;
    vector<QString> aliases() const override;

    ushort usage() const {return usage_;}
    const QString &url() const {return url_;}

private:
    QString      name_;
    QString      url_;
    ushort       usage_;
    static QIcon icon_;
};
}
