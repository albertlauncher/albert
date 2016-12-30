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
#include "extensionmanager.h"
#include "extensionspec.h"
using std::unique_ptr;
using std::chrono::system_clock;

namespace {
const QString CFG_BLACKLIST = "blacklist";
}

/** ***************************************************************************/
Core::ExtensionManager::ExtensionManager() {
    // Load blacklist
    blacklist_ = QSettings(qApp->applicationName()).value(CFG_BLACKLIST).toStringList();
    reloadExtensions();
}



/** ***************************************************************************/
Core::ExtensionManager::~ExtensionManager() {
    for (unique_ptr<ExtensionSpec> & extensionSpec : extensionSpecs_)
        unloadExtension(extensionSpec);
}



/** ***************************************************************************/
void Core::ExtensionManager::reloadExtensions() {

    // Unload everything
    for (unique_ptr<ExtensionSpec> & extensionSpec : extensionSpecs_)
        unloadExtension(extensionSpec);

    extensionSpecs_.clear();

    // Get plugindirs
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

                QPluginLoader loader(path);

                // Check for a sane interface ID  (IID)
                QString iid = loader.metaData()["IID"].toString();
                if (iid != ALBERT_EXTENSION_IID)
                    continue;

                // Check for duplicates
                QString id = loader.metaData()["MetaData"].toObject()["id"].toString();
                if (std::find_if (extensionSpecs_.begin(), extensionSpecs_.end(),
                                  [&id](const unique_ptr<ExtensionSpec> & extensionSpec){
                                      return id == extensionSpec->id();
                                  }) != extensionSpecs_.end())
                    continue;

                // Put it to the results
                extensionSpecs_.emplace_back(new ExtensionSpec(path));
            }
        }
    }

    // Load if not blacklisted
    for (unique_ptr<ExtensionSpec> & extensionSpec : extensionSpecs_)
        if (!blacklist_.contains(extensionSpec->id()))
            loadExtension(extensionSpec);
}



/** ***************************************************************************/
const vector<unique_ptr<Core::ExtensionSpec>>& Core::ExtensionManager::extensionSpecs() const {
    return extensionSpecs_;
}



/** ***************************************************************************/
const set<Core::Extension*> Core::ExtensionManager::extensions() const {
    return extensions_;
}



/** ***************************************************************************/
void Core::ExtensionManager::loadExtension(const unique_ptr<ExtensionSpec> &spec) {
    if (spec->state() != ExtensionSpec::State::Loaded){
        system_clock::time_point start = system_clock::now();
        if ( spec->load() ) {
            auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now()-start);
            qDebug() << QString("Loading %1 done in %2 milliseconds").arg(spec->id()).arg(msecs.count());
            extensions_.insert(spec->instance());
        } else
            qDebug() << QString("Loading %1 failed. (%2)").arg(spec->id(), spec->lastError());
    }
}



/** ***************************************************************************/
void Core::ExtensionManager::unloadExtension(const unique_ptr<ExtensionSpec> &spec) {
    if (spec->state() == ExtensionSpec::State::Loaded) {
        extensions_.erase(spec->instance());
        spec->unload();
    }
}



/** ***************************************************************************/
void Core::ExtensionManager::enableExtension(const unique_ptr<ExtensionSpec> &spec) {
    blacklist_.removeAll(spec->id());
    QSettings(qApp->applicationName()).setValue(CFG_BLACKLIST, blacklist_);
    loadExtension(spec);
}



/** ***************************************************************************/
void Core::ExtensionManager::disableExtension(const unique_ptr<ExtensionSpec> &spec) {
    if (!blacklist_.contains(spec->id())){
        blacklist_.push_back(spec->id());
        QSettings(qApp->applicationName()).setValue(CFG_BLACKLIST, blacklist_);
    }
    unloadExtension(spec);
}



/** ***************************************************************************/
bool Core::ExtensionManager::extensionIsEnabled(const unique_ptr<ExtensionSpec> &spec) {
    return !blacklist_.contains(spec->id());
}
