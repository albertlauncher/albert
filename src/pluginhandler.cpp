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
#include <QSettings>
#include "plugininterfaces/extension_if.h"

#define PLUGINFOLDER "plugins"

/** ***************************************************************************/
PluginHandler::PluginHandler() {
    _blacklist = QSettings().value(CFG_BLACKLIST).toStringList();
}



/** ***************************************************************************/
PluginHandler::~PluginHandler() {
    QSettings().setValue(CFG_BLACKLIST, _blacklist);
}



/** ***************************************************************************/
const QList<PluginSpec*> &PluginHandler::pluginSpecs() {
    return _plugins;
}



/** ***************************************************************************/
void PluginHandler::loadPlugins() {
    QSettings s;
    QStringList pluginDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation,
                "plugins",
                QStandardPaths::LocateDirectory);

    qDebug() << "Loading plugins in" << pluginDirs;

    for (QString pluginDir : pluginDirs)
    {
        QDirIterator dirIterator(pluginDir, QDir::Files);
        while (dirIterator.hasNext())
        {
            QString path = dirIterator.next();

            // Check if this is a lib
            if (!QLibrary::isLibrary(path)) {
                qDebug() << "Not a library:" << path;
                continue;
            }

            // Fill pluginspec
            PluginSpec *ps = new PluginSpec;
            ps->loader = new QPluginLoader(path);
            QJsonObject metaData = ps->loader->metaData()["MetaData"].toObject();
            ps->path = path;
            ps->IID = ps->loader->metaData()["IID"].toString();
            ps->id = metaData["id"].toString();
            ps->name = metaData["name"].toString();
            ps->version = metaData["version"].toString();
            ps->platform = metaData["platform"].toString();
            ps->group = metaData["group"].toString();
            ps->copyright = metaData["copyright"].toString();
            ps->description = metaData["description"].toString();
            for (const QJsonValue &v : metaData["dependencies"].toArray()){
                QString dep = v.toString();
                if (!dep.isEmpty()) // hä?
                    ps->dependencies << dep; // TODO CHECK THEM
            }

            // Check if this lib is an albert extension plugin
            if (! ps->loader->metaData()["IID"].toString().compare(ALBERT_EXTENSION_IID)==0 ){
                qWarning() << "Extension incompatible:" << path << ps->IID << ALBERT_EXTENSION_IID;
                delete ps->loader;
                delete ps;
                continue;
            }

            // Store the plugin
            _plugins.append(ps);

            // Check if this extension is blacklisted
            if (s.value(CFG_BLACKLIST).toStringList().contains(ps->name)){
                qDebug() << "Extension blacklisted:" << path;
                ps->status = PluginSpec::Status::NotLoaded;
                continue;
            }

            // Load the plugin
            if (!ps->loader->load()){
                qWarning() << "WARNING: Loading extension failed:" << path << ps->loader->errorString();
                ps->status = PluginSpec::Status::Error;
                continue;
            }

            qDebug() << "Extension loaded:" <<  path;
            ps->status = PluginSpec::Status::Loaded;
            emit pluginLoaded(ps->loader->instance());
        }
    }
}



/** ***************************************************************************/
void PluginHandler::unloadPlugins() {
    for (PluginSpec *ps :_plugins){
        emit pluginAboutToBeUnloaded(ps->loader->instance()); // THIS HAS TO BE BLOCKING
        ps->loader->unload();
        delete ps->loader;
        delete ps;
    }
}



/** ***************************************************************************/
QStringList &PluginHandler::blacklist(){
    return _blacklist;
}
