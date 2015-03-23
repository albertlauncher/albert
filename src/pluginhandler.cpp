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

#include "pluginhandler.h"
#include <QDirIterator>
#include <QDebug>
#include <QStandardPaths>
#include "settings.h"
#include "extensioninterface.h"

#define PLUGINFOLDER "plugins"

/****************************************************************************///
PluginHandler *PluginHandler::_instance = nullptr;
/****************************************************************************///
PluginHandler *PluginHandler::instance() {
    if( _instance == nullptr )
        _instance = new PluginHandler();
    return _instance;
}

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
        QStringList blacklist = gSettings->value(SETTINGS_PLGN_BLACKLIST).toStringList();
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
            ps.IID = loader->metaData()["IID"].toString();
            ps.name = metaData["name"].toString();
            ps.path = path;
            ps.version = metaData["version"].toString();
            ps.platform = metaData["platform"].toString();
            ps.group = metaData["group"].toString();
            ps.dependencies = metaData["dependencies"].toString().split(", " , QString::SkipEmptyParts);
            ps.description = metaData["description"].toString();
            ps.loader = loader;

            // Check if this lib is an albert extension plugin
            if (loader->metaData()["IID"].toString().compare(ALBERT_EXTENSION_IID) != 0) {
                qWarning() << "Extension incompatible:" << path << ps.IID << ALBERT_EXTENSION_IID;
                ps.status = PluginSpec::Status::Error;
                delete loader;
                continue;
            }

            // Check if this extension is blacklisted
            if (blacklist.contains(ps.name)){
                qWarning() << "WARNING: Extension blacklisted:" << path;
                ps.status = PluginSpec::Status::Blacklisted;
                delete loader;
                continue;
            }

            // Load the plugin
            if (!loader->load()){
                qWarning() << "WARNING: Loading extension failed:" << path << loader->errorString();
                ps.status = PluginSpec::Status::Error;
                delete loader;
                continue;
            }
            ps.status = PluginSpec::Status::Loaded;

            // Store the plugins
            _plugins.append(ps);
            qDebug() << "Extension loaded:" <<  path;
        }
    }
}
