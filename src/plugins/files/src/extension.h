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
#include <QObject>
#include <memory>
#include "core/extension.h"
#include "core/queryhandler.h"

namespace Files {

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

    QString name() const override { return "Files"; }
    QStringList triggers() const override { return {"/", "~"}; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) override;

    const QStringList &paths() const;
    void setPaths(const QStringList &);
    void restorePaths();

    bool indexHidden() const;
    void setIndexHidden(bool b = true);

    bool followSymlinks() const;
    void setFollowSymlinks(bool b = true);

    uint scanInterval() const;
    void setScanInterval(uint minutes);

    bool fuzzy() const;
    void setFuzzy(bool b = true);

    const QStringList &filters() const;
    void setFilters(const QStringList &);

    void updateIndex();

private:

    std::unique_ptr<Private> d;

signals:

    void rootDirsChanged(const QStringList&);
    void statusInfo(const QString&);

};
}
