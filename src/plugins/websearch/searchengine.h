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
#include "abstractobjects.h"
class Extension;

namespace Websearch {

class SearchEngine final : public A2Leaf
{
public:
    SearchEngine() : enabled_(false) {}
    SearchEngine(QString name, QString url, QString trigger, QString iconPath, bool enabled = true);

    QString text() const override;
    QString subtext() const override;
    QIcon icon() const override;
    void activate() override;

    QString trigger();
    void serialize(QDataStream &out);
    void deserialize(QDataStream &in);

    bool enabled() const {return enabled_;}
    void setEnabled(bool b) {enabled_ = b;}

    QString query() const {return searchTerm_;}
    void setQuery(QString b) {searchTerm_ = b;}

    QString name() const {return name_;}
    QString trigger() const {return trigger_;}
    QString url() const {return url_;}


private:
    bool    enabled_;
    QString name_;
    QString url_;
    QString trigger_;
    QString searchTerm_;
    QString iconPath_;
    QIcon   icon_;
};

}
