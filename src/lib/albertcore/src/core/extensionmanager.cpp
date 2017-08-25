// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QLibrary>
#include <QPluginLoader>
#include <QSettings>
#include <QStandardPaths>
#include <chrono>
#include "extensionmanager.h"
#include "pluginspec.h"
using std::set;
using std::unique_ptr;
using std::vector;


/** ***************************************************************************/
class Core::ExtensionManagerPrivate {
public:
    vector<unique_ptr<PluginSpec>> extensionSpecs_;
    set<QObject*> extensions_;
};


/** ***************************************************************************/
Core::ExtensionManager::ExtensionManager(QStringList pluginDirs)
    : d(new ExtensionManagerPrivate) {

    // Find plugins
    for ( const QString &pluginDir : pluginDirs ) {
       QDirIterator dirIterator(pluginDir, QDir::Files);
       while (dirIterator.hasNext()) {
           std::unique_ptr<PluginSpec> plugin(new PluginSpec(dirIterator.next()));

           if ( plugin->iid() != ALBERT_EXTENSION_IID )
               continue;

           if (std::any_of(d->extensionSpecs_.begin(), d->extensionSpecs_.end(),
                           [&](const unique_ptr<PluginSpec> &spec){ return plugin->id() == spec->id(); })) {
               qWarning() << qPrintable(QString("Frontend IDs already exists. Skipping. (%1)").arg(plugin->path()));
               continue;
           }

           d->extensionSpecs_.push_back(std::move(plugin));
       }
    }

    // Sort alphabetically
    std::sort(d->extensionSpecs_.begin(),
              d->extensionSpecs_.end(),
              [](const unique_ptr<PluginSpec>& lhs, const unique_ptr<PluginSpec>& rhs){
        return lhs->name() < rhs->name();
    });
}


/** ***************************************************************************/
Core::ExtensionManager::~ExtensionManager() {
    for (unique_ptr<PluginSpec> & pluginSpec : d->extensionSpecs_)
        unloadExtension(pluginSpec);
}


/** ***************************************************************************/
const vector<unique_ptr<Core::PluginSpec>>& Core::ExtensionManager::extensionSpecs() const {
    return d->extensionSpecs_;
}


/** ***************************************************************************/
void Core::ExtensionManager::reloadExtensions() {

    // Unload all extensions
    for (unique_ptr<PluginSpec> & pluginSpec : d->extensionSpecs_)
        unloadExtension(pluginSpec);

    // Load if enabled
    QSettings settings(qApp->applicationName());
    for (unique_ptr<PluginSpec> & pluginSpec : d->extensionSpecs_) {
        if ( settings.value(QString("%1/enabled").arg(pluginSpec->id()), false).toBool() )
            loadExtension(pluginSpec);
    }

}


/** ***************************************************************************/
const set<QObject*> Core::ExtensionManager::objects() const {
    return d->extensions_;
}


/** ***************************************************************************/
void Core::ExtensionManager::loadExtension(const unique_ptr<PluginSpec> &spec) {
    if ( spec->state() != PluginSpec::State::Loaded ){
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        if ( spec->load() ) {
            d->extensions_.insert(spec->instance());
            auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-start);
            qDebug() << qPrintable(QString("%1 loaded in %2 milliseconds").arg(spec->id()).arg(msecs.count()));
        } else
            qInfo() << QString("Loading %1 failed. (%2)").arg(spec->id(), spec->lastError()).toLocal8Bit().data();
    }
}


/** ***************************************************************************/
void Core::ExtensionManager::unloadExtension(const unique_ptr<PluginSpec> &spec) {
    if (spec->state() != PluginSpec::State::NotLoaded) {
        d->extensions_.erase(spec->instance());
        spec->unload();
    }
}


/** ***************************************************************************/
void Core::ExtensionManager::enableExtension(const unique_ptr<PluginSpec> &pluginSpec) {
    QSettings(qApp->applicationName()).setValue(QString("%1/enabled").arg(pluginSpec->id()), true);
    loadExtension(pluginSpec);
}


/** ***************************************************************************/
void Core::ExtensionManager::disableExtension(const unique_ptr<PluginSpec> &pluginSpec) {
    QSettings(qApp->applicationName()).setValue(QString("%1/enabled").arg(pluginSpec->id()), false);
    unloadExtension(pluginSpec);
}


/** ***************************************************************************/
bool Core::ExtensionManager::extensionIsEnabled(const unique_ptr<PluginSpec> &pluginSpec) {
    QSettings settings(qApp->applicationName());
    return settings.value(QString("%1/enabled").arg(pluginSpec->id())).toBool();
}


/** ***************************************************************************/
void Core::ExtensionManager::registerObject(QObject *object) {
    d->extensions_.insert(object);
}


/** ***************************************************************************/
void Core::ExtensionManager::unregisterObject(QObject *object) {
    d->extensions_.erase(object);
}
