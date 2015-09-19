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
#include <QMap>
#include <QString>
#include "pluginloader.h"


class PluginManager final : public QObject {
    Q_OBJECT

public:
    PluginManager();
    ~PluginManager();

    void loadPlugins();
    void unloadPlugins();
    const QMap<QString, PluginLoader*> & plugins();

    void enable(const QString &path);
    void disable(const QString &path);
    bool isEnabled(const QString &path);

private:
    QMap<QString, PluginLoader*> _plugins;
    QStringList _blacklist;
    static const constexpr char* CFG_BLACKLIST = "blacklist";

signals:
    void pluginLoaded(QObject*);
    void pluginAboutToBeUnloaded(QObject*);
};
