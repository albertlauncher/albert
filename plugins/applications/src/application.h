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
#include "objects.h"

namespace Applications {

class Application : public AlbertObject {

    friend class Extension;

public:
    Application();

    QString name() override;
    QString description() override;
    QStringList alises() override;
    QIcon icon() override;
    uint usage() override;
    QList<std::shared_ptr<Action>> actions() override;

private:
    QString _path;
    QString _name;
    QString _altName;
    QString _exec;
    QIcon _icon;
    uint _usage;
    static QHash<QString, QIcon> _iconCache;
};
typedef QSharedPointer<Application> SharedApp;
}
