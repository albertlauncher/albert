// albert - a simple application launcher for linux
// Copyright (C) 2016-2017 Martin Buergmann
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
#include <QObject>
#include "core/extension.h"
#include "core/queryhandler.h"
#include <memory>

namespace FirefoxBookmarks {

class Private;

class Extension final :
        public Core::Extension,
        public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension();

    QString name() const override { return "Firefox bookmarks"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) override;

    void setProfile(const QString &profile);
    void changeFuzzyness(bool fuzzy);
    void changeOpenPolicy(bool withFirefox);

private:

    std::unique_ptr<Private> d;

signals:

    void statusInfo(const QString&);
};
}
