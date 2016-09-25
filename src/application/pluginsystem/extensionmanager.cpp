// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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

#include <QDirIterator>
#include <QDebug>
#include <QStandardPaths>
#include <QSettings>
#include <chrono>
#include "extensionmanager.h"
#include "albertapp.h"

const QString ExtensionManager::CFG_BLACKLIST = "blacklist";

/** ***************************************************************************/
ExtensionManager::ExtensionManager() {
    // Load settings
    blacklist_ = qApp->settings()->value(CFG_BLACKLIST).toStringList();

    // Iterate over all files in the plugindirs
    QStringList pluginDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation,
                "plugins",
                QStandardPaths::LocateDirectory);
    for (const QString &pluginDir : pluginDirs) {
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
}



/** ***************************************************************************/
ExtensionManager::~ExtensionManager() {
    qDebug() << "[ExtensionManager] Unloading plugins.";
    for (const unique_ptr<PluginSpec> &plugin : plugins_){
        unloadPlugin(plugin);
    }
    qDebug() << "[ExtensionManager] Unloading plugins done.";
}



/** ***************************************************************************/
const vector<unique_ptr<PluginSpec> > &ExtensionManager::plugins() {
    return plugins_;
}



/** ***************************************************************************/
void ExtensionManager::loadPlugin(const unique_ptr<PluginSpec> &plugin) {
    if (!plugin->isLoaded()){

        auto start = std::chrono::system_clock::now();
        plugin->load();

        // Test for success and propagate this
        if (plugin->status() == PluginSpec::Status::Loaded) {
            auto now = std::chrono::system_clock::now();
            auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(now-start);
            qDebug("Loading %s done in %d milliseconds", plugin->id().toLocal8Bit().data(), (int)msecs.count());
            emit pluginLoaded(plugin->instance());
        } else
            qDebug("Loading %s failed. (%s)", plugin->id().toLocal8Bit().data(), plugin->errorString().toLocal8Bit().data());
    }
}



/** ***************************************************************************/
void ExtensionManager::unloadPlugin(const unique_ptr<PluginSpec> &plugin) {
    if (plugin->isLoaded()){
        emit pluginAboutToBeUnloaded(plugin->instance());
        plugin->unload();
    }
}



/** ***************************************************************************/
void ExtensionManager::enablePlugin(const unique_ptr<PluginSpec> &plugin) {
    blacklist_.removeAll(plugin->id());
    qApp->settings()->setValue(CFG_BLACKLIST, blacklist_);
    loadPlugin(plugin);
}



/** ***************************************************************************/
void ExtensionManager::disablePlugin(const unique_ptr<PluginSpec> &plugin) {
    if (!blacklist_.contains(plugin->id())){
        blacklist_.push_back(plugin->id());
        qApp->settings()->setValue(CFG_BLACKLIST, blacklist_);
    }
    unloadPlugin(plugin);
}



/** ***************************************************************************/
bool ExtensionManager::pluginIsEnabled(const unique_ptr<PluginSpec> &plugin) {
    return !blacklist_.contains(plugin->id());
}
