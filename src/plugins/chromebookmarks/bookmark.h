// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include "iindexable.h"
#include "abstractitem.h"

namespace ChromeBookmarks {

class Bookmark final : public AbstractItem, public IIndexable
{
public:

    Bookmark() {}
    Bookmark(const QString &id, const QString &name, const QString &url)
        : id_(id), name_(name), url_(url) {}

    /*
     * Implementation of Item interface
     */

    QString id() const override { return id_; }
    QString text() const override;
    QString subtext() const override;
    QString iconPath() const override;
    vector<QString> indexKeywords() const override;
    void activate(ExecutionFlags *) override;

    /*
     * Item specific members
     */

    /** Return the name of the bookmark */
    const QString& name() const { return name_; }

    /** Sets the name of the bookmark */
    void setName(const QString& name) { name_ = name; }

    /** Return the path of the bookmark */
    const QString& url() const { return url_; }

    /** Sets the url of the bookmark */
    void setUrl(const QString& url) { url_ = url; }

private:
    QString id_;
    QString name_;
    QString url_;
};
}
