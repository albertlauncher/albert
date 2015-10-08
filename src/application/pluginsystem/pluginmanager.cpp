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

#include "pluginmanager.h"
#include <QDirIterator>
#include <QDebug>
#include <QStandardPaths>
#include <QSettings>


/** ***************************************************************************/
PluginManager::PluginManager() {
    qDebug() << "[PluginManager] Loading plugins.";

    // Load settings
    blacklist_ = QSettings().value(CFG_BLACKLIST).toStringList();

    // Iterate over all files in the plugindirs
    QStringList pluginDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation,
                "plugins",
                QStandardPaths::LocateDirectory);
    for (QString pluginDir : pluginDirs) {
        QDirIterator dirIterator(pluginDir, QDir::Files);
        while (dirIterator.hasNext()) {
            dirIterator.next();
            QString path = dirIterator.fileInfo().canonicalFilePath();

            // Check if this path is a lib
            if (!QLibrary::isLibrary(path)) {
                qWarning() << "Non-library in plugins path:" << path;
                continue;
            }

            unique_ptr<PluginSpec> plugin(new PluginSpec(path));

            // Avoid loading duplicate
            if (std::find_if (plugins_.begin(), plugins_.end(),
                              [&plugin](const unique_ptr<PluginSpec> &p){
                                  return p->id() == plugin->id();
                              }) != plugins_.end()){
                qWarning() << "Extension of same ID already loaded" << plugin->id();
                continue;
            }

            // Load if not blacklisted
            if (!blacklist_.contains(plugin->id()))
                loadPlugin(plugin);

            // Store the plugin
            plugins_.push_back(std::move(plugin));
        }
    }
    qDebug() << "[PluginManager] Loading plugins done.";
}



/** ***************************************************************************/
PluginManager::~PluginManager() {
    qDebug() << "[PluginManager] Unloading plugins.";
    QSettings().setValue(CFG_BLACKLIST, blacklist_);
    for (const unique_ptr<PluginSpec> &plugin : plugins_){
        unloadPlugin(plugin);
    }
    qDebug() << "[PluginManager] Unloading plugins done.";
}



/** ***************************************************************************/
const vector<unique_ptr<PluginSpec> > &PluginManager::plugins() {
    return plugins_;
}



/** ***************************************************************************/
void PluginManager::loadPlugin(const unique_ptr<PluginSpec> &plugin) {
    if (!plugin->isLoaded()){
        plugin->load();

        // Test for success and propagate this
        if (plugin->status() == PluginSpec::Status::Loaded) {
            qDebug() << "[Pluginloader] Plugin loaded:" <<  plugin->id();
            emit pluginLoaded(plugin->instance());
        }
    }
}



/** ***************************************************************************/
void PluginManager::unloadPlugin(const unique_ptr<PluginSpec> &plugin) {
    if (plugin->isLoaded()){
        emit pluginAboutToBeUnloaded(plugin->instance());
        plugin->unload();
    }
}



/** ***************************************************************************/
void PluginManager::enablePlugin(const unique_ptr<PluginSpec> &plugin) {
    blacklist_.removeAll(plugin->id());
    loadPlugin(plugin);
}



/** ***************************************************************************/
void PluginManager::disablePlugin(const unique_ptr<PluginSpec> &plugin) {
    if (!blacklist_.contains(plugin->id()))
        blacklist_.push_back(plugin->id());
    unloadPlugin(plugin);
}



/** ***************************************************************************/
bool PluginManager::pluginIsEnabled(const unique_ptr<PluginSpec> &plugin) {
    return !blacklist_.contains(plugin->id());
}
