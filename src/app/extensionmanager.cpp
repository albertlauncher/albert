// Copyright (C) 2014-2018 Manuel Schneider

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
#include "albert/queryhandler.h"
#include "albert/fallbackprovider.h"
#include "pluginspec.h"
using namespace std;
using namespace chrono;


/** ***************************************************************************/
class Core::ExtensionManagerPrivate {
public:
    vector<unique_ptr<PluginSpec>> extensionSpecs_;
    set<Extension*> loadedExtensions_;
    set<QueryHandler*> queryHandlers_;
    set<FallbackProvider*> fallbackProviders_;
};


/** ***************************************************************************/
Core::ExtensionManager::ExtensionManager(QStringList pluginDirs)
    : d(new ExtensionManagerPrivate) {

    Q_ASSERT( Extension::extensionManager == nullptr);
    Extension::extensionManager = this;

    // Find plugins
    for ( const QString &pluginDir : pluginDirs ) {
       QDirIterator dirIterator(pluginDir, QDir::Files);
       while (dirIterator.hasNext()) {
           std::unique_ptr<PluginSpec> plugin(new PluginSpec(dirIterator.next()));

           if ( plugin->iid() != ALBERT_EXTENSION_IID )
               continue;

           if (std::any_of(d->extensionSpecs_.begin(), d->extensionSpecs_.end(),
                           [&](const unique_ptr<PluginSpec> &spec){ return plugin->id() == spec->id(); })) {
               qWarning() << qPrintable(QString("Extension IDs already exists. Skipping. (%1)").arg(plugin->path()));
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
void Core::ExtensionManager::loadExtension(const unique_ptr<PluginSpec> &spec) {
    if ( spec->state() != PluginSpec::State::Loaded ){

        // Load
        qInfo() << "Loading extension" << spec->id();
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        if ( !spec->load() ) {
            qInfo() << QString("Loading %1 failed. (%2)").arg(spec->id(), spec->lastError()).toLocal8Bit().data();
            return;
        }
        qDebug() << qPrintable(QString("%1 loaded in %2 milliseconds").arg(spec->id())
                               .arg(duration_cast<milliseconds>(system_clock::now()-start).count()));

        Extension *extension = dynamic_cast<Extension*>(spec->instance());
        if (!extension) {
            qInfo() << QString("Instance is not of tyoe Extension. (%2)").arg(spec->id()).toLocal8Bit().data();
            return;
        }

        d->loadedExtensions_.insert(extension);
    }
}


/** ***************************************************************************/
void Core::ExtensionManager::unloadExtension(const unique_ptr<PluginSpec> &spec) {
    if (spec->state() == PluginSpec::State::NotLoaded)
        return;

    if (spec->state() == PluginSpec::State::Loaded)
        d->loadedExtensions_.erase(dynamic_cast<Extension*>(spec->instance()));

    spec->unload();
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
void Core::ExtensionManager::registerQueryHandler(Core::QueryHandler *queryHandler) {
    d->queryHandlers_.insert(queryHandler);
    emit queryHandlerRegistered(queryHandler);
}


/** ***************************************************************************/
void Core::ExtensionManager::unregisterQueryHandler(Core::QueryHandler *queryHandler) {
    d->queryHandlers_.erase(queryHandler);
    emit queryHandlerUnregistered(queryHandler);
}


/** ***************************************************************************/
const std::set<Core::QueryHandler*> &Core::ExtensionManager::queryHandlers() {
    return d->queryHandlers_;
}


/** ***************************************************************************/
void Core::ExtensionManager::registerFallbackProvider(Core::FallbackProvider *fallbackProvider) {
    d->fallbackProviders_.insert(fallbackProvider);
    emit fallbackProviderRegistered(fallbackProvider);

}


/** ***************************************************************************/
void Core::ExtensionManager::unregisterFallbackProvider(Core::FallbackProvider *fallbackProvider) {
    d->fallbackProviders_.erase(fallbackProvider);
    emit fallbackProviderUnregistered(fallbackProvider);
}


/** ***************************************************************************/
const std::set<Core::FallbackProvider*> &Core::ExtensionManager::fallbackProviders() {
    return d->fallbackProviders_;
}
