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

#include <QApplication>
#include <QDebug>
#include <QDirIterator>
#include <QLibrary>
#include <QPluginLoader>
#include <QSettings>
#include <QStandardPaths>
#include <chrono>
#include <memory>
#include "extension.h" // IID
#include "externalextensionloader.h"
#include "extensionmanager.h"
#include "nativeextensionloader.h"
using std::unique_ptr;
using std::chrono::system_clock;


const QString Core::ExtensionManager::CFG_BLACKLIST = "blacklist";


namespace Core {

    vector<unique_ptr<NativeExtensionLoader>> findNativeExtensions() {
        vector<unique_ptr<NativeExtensionLoader>> results;
        QStringList pluginDirs = QStandardPaths::locateAll(
                    QStandardPaths::DataLocation, "plugins",
                    QStandardPaths::LocateDirectory);
        // Iterate over all files in the plugindirs
        for (const QString &pluginDir : pluginDirs) {
            QDirIterator dirIterator(pluginDir, QDir::Files);
            while (dirIterator.hasNext()) {
                dirIterator.next();

                QString path = dirIterator.fileInfo().canonicalFilePath();

                // Check if this path is a lib
                if (QLibrary::isLibrary(path)) {

                    // Check for a sane interface ID  (IID)
                    QString iid = QPluginLoader(path).metaData()["IID"].toString();
                    if (iid != ALBERT_EXTENSION_IID)
                        continue;

                    // Put it to the results
                    results.emplace_back(new NativeExtensionLoader(path));
                }
            }
        }
        return results;
    }

    vector<unique_ptr<ExternalExtensionLoader>> findExternalExtensions() {
        vector<unique_ptr<ExternalExtensionLoader>> results;

        QStringList pluginDirs = QStandardPaths::locateAll(
                    QStandardPaths::DataLocation, "external",
                    QStandardPaths::LocateDirectory);

        // Iterate over all files in the plugindirs
        for (const QString &pluginDir : pluginDirs) {
            QDirIterator dirIterator(pluginDir, QDir::Files|QDir::Executable, QDirIterator::NoIteratorFlags);
            while (dirIterator.hasNext()) {
                dirIterator.next();
                try {
                    results.emplace_back(new ExternalExtensionLoader(dirIterator.fileInfo().canonicalFilePath()));
                } catch (QString error) {
                    qWarning() << error;
                    continue;
                }
            }
        }
        return results;
    }

}


/** ***************************************************************************/
Core::ExtensionManager::ExtensionManager() {
    // Load blacklist
    blacklist_ = QSettings(qApp->applicationName()).value(CFG_BLACKLIST).toStringList();
    rescanExtensions();
}



/** ***************************************************************************/
Core::ExtensionManager::~ExtensionManager() {
    for (unique_ptr<ExtensionLoader> & extensionLoader : extensionLoaders_)
        unloadExtension(extensionLoader);
}



/** ***************************************************************************/
void Core::ExtensionManager::rescanExtensions() {
    // Unload everything
    for (unique_ptr<ExtensionLoader> & extensionLoader : extensionLoaders_)
        unloadExtension(extensionLoader);

    vector<unique_ptr<ExtensionLoader>> notDistinctLoaders;

    // Load native extensions
    vector<unique_ptr<NativeExtensionLoader>> && natives = findNativeExtensions();
    std::move(natives.begin(), natives.end(), std::back_inserter(notDistinctLoaders));

    // Load external extensions
    vector<unique_ptr<ExternalExtensionLoader>> && externals = findExternalExtensions();
    std::move(externals.begin(), externals.end(), std::back_inserter(notDistinctLoaders));

    // Save extensionLoaders, drop duplicates
    for (unique_ptr<ExtensionLoader> & extensionLoader : notDistinctLoaders)
        if (std::find_if (extensionLoaders_.begin(), extensionLoaders_.end(),
                          [&extensionLoader](const unique_ptr<ExtensionLoader> &other){
                              return extensionLoader->id() == other->id();
                          }) != extensionLoaders_.end())
            continue;
        else
            extensionLoaders_.push_back(std::move(extensionLoader));

    // Load if not blacklisted
    for (unique_ptr<ExtensionLoader> & extensionLoader : extensionLoaders_)
        if (!blacklist_.contains(extensionLoader->id()))
            loadExtension(extensionLoader);
}



/** ***************************************************************************/
const vector<unique_ptr<Core::ExtensionLoader>>& Core::ExtensionManager::extensionLoaders() const {
    return extensionLoaders_;
}



/** ***************************************************************************/
set<Core::Extension*> Core::ExtensionManager::extensions() const {
    return extensions_;
}



/** ***************************************************************************/
void Core::ExtensionManager::loadExtension(const unique_ptr<ExtensionLoader> &loader) {
    if (loader->state() != ExtensionLoader::State::Loaded){
        system_clock::time_point start = system_clock::now();
        if ( loader->load() ) {
            auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now()-start);
            qDebug() << QString("Loading %1 done in %2 milliseconds").arg(loader->id()).arg(msecs.count());
            extensions_.insert(loader->instance());
        } else
            qDebug() << QString("Loading %1 failed. (%2)").arg(loader->id(), loader->lastError());
    }
}



/** ***************************************************************************/
void Core::ExtensionManager::unloadExtension(const unique_ptr<ExtensionLoader> &loader) {
    if (loader->state() == ExtensionLoader::State::Loaded) {
        extensions_.erase(loader->instance());
        loader->unload();
    }
}



/** ***************************************************************************/
void Core::ExtensionManager::enableExtension(const unique_ptr<ExtensionLoader> &loader) {
    blacklist_.removeAll(loader->id());
    QSettings(qApp->applicationName()).setValue(CFG_BLACKLIST, blacklist_);
    loadExtension(loader);
}



/** ***************************************************************************/
void Core::ExtensionManager::disableExtension(const unique_ptr<ExtensionLoader> &loader) {
    if (!blacklist_.contains(loader->id())){
        blacklist_.push_back(loader->id());
        QSettings(qApp->applicationName()).setValue(CFG_BLACKLIST, blacklist_);
    }
    unloadExtension(loader);
}



/** ***************************************************************************/
bool Core::ExtensionManager::extensionIsEnabled(const unique_ptr<ExtensionLoader> &loader) {
    return !blacklist_.contains(loader->id());
}
