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

#include "pluginhandler.h"
#include <QDirIterator>
#include <QDebug>
#include <QJsonArray>
#include <QStandardPaths>
#include "settings.h"
#include "plugininterfaces/extensioninterface.h"

#define PLUGINFOLDER "plugins"

/****************************************************************************///
void PluginHandler::loadPlugins()
{
    QStringList pluginDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation,
                "plugins",
                QStandardPaths::LocateDirectory);

    qDebug() << "Loading plugins in" << pluginDirs;

    for (QString pd : pluginDirs)
    {
        QDirIterator plgnIt(pd, QDir::Files);
        while (plgnIt.hasNext())
        {
            QString path = plgnIt.next();

            // Check if this is a lib
            if (!QLibrary::isLibrary(path)) {
                qDebug() << "Not a library:" << path;
                continue;
            }

            QPluginLoader *loader = new QPluginLoader(path);
            QJsonObject metaData = loader->metaData()["MetaData"].toObject();
            PluginSpec ps;
            ps.path = path;
            ps.IID = loader->metaData()["IID"].toString();
            ps.name = metaData["name"].toString();
            ps.version = metaData["version"].toString();
            ps.platform = metaData["platform"].toString();
            ps.group = metaData["group"].toString();
            ps.copyright = metaData["copyright"].toString();
            ps.description = metaData["description"].toString();
            for (const QJsonValue &v : metaData["dependencies"].toArray()){
                QString dep = v.toString();
                if (!dep.isEmpty())
                    ps.dependencies << dep;
            }
            ps.loader = loader;

            // Check if this lib is an albert extension plugin
            if (loader->metaData()["IID"].toString().compare(ALBERT_EXTENSION_IID) != 0) {
                qWarning() << "Extension incompatible:" << path << ps.IID << ALBERT_EXTENSION_IID;
                ps.status = PluginSpec::Status::Error;
                delete loader;
                continue;
            }

            // Check if this extension is blacklisted
            if (gSettings->value(CFG_BLACKLIST).toStringList().contains(ps.name)){
                qWarning() << "WARNING: Extension blacklisted:" << path;
                ps.status = PluginSpec::Status::NotLoaded;
            }
            else
            {
                // Load the plugin
                if (!loader->load()){
                    qWarning() << "WARNING: Loading extension failed:" << path << loader->errorString();
                    ps.status = PluginSpec::Status::Error;
                }else{
                    qDebug() << "Extension loaded:" <<  path;
                    ps.status = PluginSpec::Status::Loaded;
                }
            }

            // Store the plugin
            _plugins.append(ps);
        }
    }
}
