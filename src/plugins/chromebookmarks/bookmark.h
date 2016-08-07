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

    Bookmark() : usage_(0) {}
    Bookmark(const QString &id, const QString &name, const QString &url, short usage = 0)
        : id_(id), name_(name), url_(url), usage_(usage) {}

    /*
     * Implementation of Item interface
     */

    QString id() const override { return id_; }
    QString text() const override;
    QString subtext() const override;
    QString iconPath() const override;
    vector<QString> indexKeywords() const override;
    void activate(ExecutionFlags *) override;
    uint16_t usageCount() const override {return usage_;}

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

    /** Return the usage count of the bookmark */
    uint16_t usage() const { return usage_; }

    /** Sets the usage count of the bookmark */
    void setUsage(uint16_t usage) { usage_ = usage; }

    /** Serialize the desktop entry */
    void serialize(QDataStream &out);

    /** Deserialize the desktop entry */
    void deserialize(QDataStream &in);

private:
    QString id_;
    QString name_;
    QString url_;
    mutable uint16_t usage_;
};
}
