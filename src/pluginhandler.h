// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
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

#ifndef PLUGINHANDLER_H
#define PLUGINHANDLER_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QPluginLoader>

struct PluginSpec
{
    enum class Status{NotLoaded, Error, Loaded, Blacklisted};
    QString path;
    QString name;
    QString IID;
    QString version;
    QString platform;
    QString group;
    QStringList dependencies;
    QString description;
    Status  status;
    QPluginLoader * loader;
};

class PluginHandler final
{
public:
    ~PluginHandler(){}
    static PluginHandler *instance();
    const QList<PluginSpec> & getPluginSpecs(){return _plugins;}

    template<typename T>
    QList<T> getLoadedPlugins(){
        QList<T> res;
        for (PluginSpec& ps : _plugins)
            if (ps.status == PluginSpec::Status::Loaded && dynamic_cast<T>(ps.loader->instance()))
                res.append(dynamic_cast<T>(ps.loader->instance()));
        return res;
    }

private:
    PluginHandler() {
        loadPlugins();
    }
    void loadPlugins();

    static PluginHandler *_instance;
    QList<PluginSpec> _plugins;
};

#endif // PLUGINHANDLER_H
