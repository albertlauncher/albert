// Copyright (C) 2014-2018 Manuel Schneider

#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QDirIterator>
#include <vector>
#include <memory>
#include "frontendmanager.h"
#include "albert/frontend.h"
#include "pluginspec.h"
using std::unique_ptr;
using std::vector;


namespace {
const char* CFG_FRONTEND_ID = "frontendId";
const char* DEF_FRONTEND_ID = "org.albert.frontend.widgetboxmodel";
}


/** ***************************************************************************/
class Core::FrontendManagerPrivate {
public:
    vector<unique_ptr<PluginSpec>> frontendPlugins;
    Frontend* currentFrontend;
};


/** ***************************************************************************/
Core::FrontendManager::FrontendManager(QStringList pluginDirs)
    : d(new FrontendManagerPrivate) {

    // Find plugins
    for ( const QString &pluginDir : pluginDirs ) {
       QDirIterator dirIterator(pluginDir, QDir::Files);
       while (dirIterator.hasNext()) {
           std::unique_ptr<PluginSpec> plugin(new PluginSpec(dirIterator.next()));

           if ( plugin->iid() != ALBERT_FRONTEND_IID )
               continue;

           if (std::any_of(d->frontendPlugins.begin(), d->frontendPlugins.end(),
                           [&](const unique_ptr<PluginSpec> &spec){ return plugin->id() == spec->id(); })) {
               qWarning() << qPrintable(QString("Frontend IDs already exists. Skipping. (%1)").arg(plugin->path()));
               continue;
           }

           d->frontendPlugins.push_back(std::move(plugin));
       }
    }

    if ( d->frontendPlugins.empty() )
        qFatal("No frontends available");

    // Find the configured frontend, fallback to default if not configured
    QSettings s(qApp->applicationName());
    QString id = s.value(CFG_FRONTEND_ID, DEF_FRONTEND_ID).toString();
    vector<unique_ptr<PluginSpec>>::iterator it =
            std::find_if(d->frontendPlugins.begin(), d->frontendPlugins.end(),
                         [&](std::unique_ptr<PluginSpec> &plugin){
            return plugin->id() == id;
    });

    if ( it == d->frontendPlugins.end() ) {
        it = d->frontendPlugins.begin();
        qWarning("Frontend '%s' could not be found. Using %s instead.",
                 qPrintable(id), qPrintable((*it)->id()));
        s.setValue(CFG_FRONTEND_ID, (*it)->id());
    }

    if ( !(*it)->load() )
        qFatal("Failed loading frontend");

    d->currentFrontend = dynamic_cast<Frontend*>((*it)->instance());
    if (!d->currentFrontend)
        qFatal("Could not cast plugin instance to frontend");
}


/** ***************************************************************************/
Core::FrontendManager::~FrontendManager() {

}



/** ***************************************************************************/
const std::vector<std::unique_ptr<Core::PluginSpec> > &Core::FrontendManager::frontendSpecs() const {
    return d->frontendPlugins;
}



/** ***************************************************************************/
Core::Frontend *Core::FrontendManager::currentFrontend() {
    return d->currentFrontend;
}



/** ***************************************************************************/
bool Core::FrontendManager::setCurrentFrontend(QString id) {

    QSettings s(qApp->applicationName());
    s.setValue(CFG_FRONTEND_ID, id);

    // Find plugin
    vector<unique_ptr<PluginSpec>>::iterator it =
            std::find_if(d->frontendPlugins.begin(), d->frontendPlugins.end(),
                         [&](std::unique_ptr<Core::PluginSpec> &spec){
            return spec->id() == id;
    });
    if ( it == d->frontendPlugins.end() ) {
        qWarning("Frontend '%s' could not be found.", qPrintable(id));
        return false;
    }
    std::unique_ptr<Core::PluginSpec> &newFrontendSpec = *it;

    it = std::find_if(d->frontendPlugins.begin(), d->frontendPlugins.end(),
                      [&](std::unique_ptr<Core::PluginSpec> &spec){
            return spec->id() == d->currentFrontend->id();
    });
    std::unique_ptr<Core::PluginSpec> &oldFrontendSpec = *it;

    newFrontendSpec->load();

    Frontend *newFrontend = dynamic_cast<Frontend*>(newFrontendSpec->instance());
    if (!newFrontend)
        qFatal("Could not cast plugin instance to frontend");

    d->currentFrontend->disconnect();
    d->currentFrontend = newFrontend;
    emit frontendChanged(d->currentFrontend);
    oldFrontendSpec->unload();

    return true;
}
