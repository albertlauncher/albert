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
#include <QList>
#include <QString>
#include <QStringList>
#include <QPluginLoader>

struct PluginSpec {
    enum class Status{NotLoaded, Error, Loaded};
    QString path;
    Status  status;
    QPluginLoader * loader;
    QString IID;
    QString id;
    QString name;
    QString version;
    QString platform;
    QString group;
    QString copyright;
    QString description;
    QStringList dependencies;
};

class PluginHandler final : public QObject {
    Q_OBJECT

public:
    PluginHandler();
    ~PluginHandler();

    const QList<PluginSpec*> & pluginSpecs();
    void loadPlugins();
    void unloadPlugins();
    QStringList &blacklist();

    template<class T>
    QList<T> getLoadedPlugins(){
        QList<T> res;
        for (PluginSpec& ps : _plugins)
            if (ps.status == PluginSpec::Status::Loaded && dynamic_cast<T>(ps.loader->instance()))
                res.append(dynamic_cast<T>(ps.loader->instance()));
        return res;
    }

private:
    QList<PluginSpec*> _plugins;
    QStringList _blacklist;

    static const constexpr char* CFG_BLACKLIST = "blacklist";

signals:
    void pluginLoaded(QObject*);
    void pluginAboutToBeUnloaded(QObject*);
};
