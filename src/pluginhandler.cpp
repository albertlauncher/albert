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
#include <QStandardPaths>
#include <QSettings>


/** ***************************************************************************/
PluginHandler::PluginHandler() {
    _blacklist = QSettings().value(CFG_BLACKLIST).toStringList();

}



/** ***************************************************************************/
PluginHandler::~PluginHandler() {
    QSettings().setValue(CFG_BLACKLIST, _blacklist);
}



/** ***************************************************************************/
const QMap<QString, PluginLoader *> &PluginHandler::plugins() {
    return _plugins;
}



/** ***************************************************************************/
void PluginHandler::enable(const QString &path){
    _blacklist.removeAll(path);
}



/** ***************************************************************************/
void PluginHandler::disable(const QString &path){
    if (!_blacklist.contains(path))
        _blacklist.append(path);
}


/** ***************************************************************************/
bool PluginHandler::isEnabled(const QString &path){
    return !_blacklist.contains(path);
}



/** ***************************************************************************/
void PluginHandler::loadPlugins() {
    qDebug() << "Loading plugins";

    // Iterate overall files in the plugindirs
    QStringList pluginDirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, "plugins", QStandardPaths::LocateDirectory);
    for (QString pluginDir : pluginDirs) {
        QDirIterator dirIterator(pluginDir, QDir::Files);
        while (dirIterator.hasNext()) {
            QString path = dirIterator.next();

            // Check if this path is a lib
            if (!QLibrary::isLibrary(path)) {
                qWarning() << "Not a library:" << path;
                continue;
            }

//            // Check if this lib is an albert extension plugin
//            if (! ps->loader->metaData()["IID"].toString().compare(ALBERT_EXTENSION_IID)==0 ){
//                qWarning() << "Extension incompatible:" << path << ps->IID << ALBERT_EXTENSION_IID;
//                delete ps->loader;
//                delete ps;
//                continue;
//            }

            PluginLoader *plugin = new PluginLoader(path);

            // Store the plugin
            _plugins.insert(path, plugin);

            // Load if not blacklisted
            if (_blacklist.contains(path))
                continue;

            plugin->load();

            // Test for success and propagate this
            if (plugin->status() == PluginLoader::Status::Loaded){
                qDebug() << "Extension loaded:" <<  plugin->name();
                emit pluginLoaded(plugin->instance());
            }
        }
    }
}



/** ***************************************************************************/
void PluginHandler::unloadPlugins() {
    for (PluginLoader *plugin : _plugins){
        if (plugin->status() == PluginLoader::Status::Loaded){
            emit pluginAboutToBeUnloaded(plugin->instance()); // THIS HAS TO BE BLOCKING
            plugin->unload();
            delete plugin;
        }
    }
}
