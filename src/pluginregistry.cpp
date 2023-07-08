// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extension/pluginprovider/pluginprovider.h"
#include "albert/logging.h"
#include "pluginregistry.h"
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
using namespace albert;
using namespace std;


PluginRegistry::PluginRegistry(ExtensionRegistry &registry)
    : albert::ExtensionWatcher<PluginProvider>(&registry), extension_registry(registry) {}

PluginRegistry::~PluginRegistry() = default; // private PluginLoader dtor

map<QString, PluginLoader*> PluginRegistry::plugins() const { return registered_plugins_; }

bool PluginRegistry::isEnabled(const QString &id) const
{ return albert::settings()->value(QString("%1/enabled").arg(id), false).toBool(); }

void PluginRegistry::enable(const QString &id, bool enable)
{
    if (isEnabled(id) != enable){
        if (auto err = load(id, enable); err.isNull())
            albert::settings()->setValue(QString("%1/enabled").arg(id), enable);
        else{
            WARN << err;
            QMessageBox::warning(nullptr, qApp->applicationDisplayName(), err);
        }
    }
}

QString PluginRegistry::load(const QString &id, bool load)
{
    try {
        if (auto *loader = registered_plugins_.at(id); load)
            return loader->load(&extension_registry);
        else
            return loader->unload(&extension_registry);
    } catch (const out_of_range&) {
        return QString("Plugin '%1' does not exist.").arg(id);
    }
}

void PluginRegistry::onAdd(PluginProvider *pp)
{
    const auto &plugins = plugins_.emplace(pp, pp->plugins()).first->second;

    for (auto &loader : plugins){

        auto id = loader->metaData().id;

        // Register plugins enforcing unique ids
        if (const auto &[it, success] = registered_plugins_.emplace(id, loader); success){

            // Load if is enabled user plugin
            if (loader->metaData().user && isEnabled(id))
                loader->load(&extension_registry);

        } else
            INFO << "Plugin" << id << "shadowed:" << loader->path;
    }

    emit pluginsChanged();
}

void PluginRegistry::onRem(PluginProvider *pp)
{
    for (auto it = registered_plugins_.begin(); it != registered_plugins_.end();){
        const auto &[id, loader] = *it;

        if (&loader->provider() == pp){

            loader->disconnect();

            // Unload plugin if necessary
            if (loader->state() != PluginState::Unloaded){

                // Wait as long as plugin is busy
                QEventLoop loop;
                connect(loader, &PluginLoader::stateChanged, &loop, &QEventLoop::quit);
                while (loader->state() == PluginState::Busy)
                    loop.exec();

                // If loaded
                if (loader->state() == PluginState::Loaded){
                    loader->unload(&extension_registry);

                    // Wait for unloading to finish
                    while (loader->state() == PluginState::Busy)
                        loop.exec();
                }
            }

            Q_ASSERT(loader->state() == PluginState::Unloaded);

            it = registered_plugins_.erase(it);
        } else
            ++it;
    }

    emit pluginsChanged();
}
