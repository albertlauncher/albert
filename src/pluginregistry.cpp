// Copyright (c) 2023 Manuel Schneider

#include "albert/extensions/pluginprovider.h"
#include "albert/logging.h"
#include "pluginregistry.h"
#include <QCoreApplication>
#include <QSettings>
#include <chrono>
using namespace std;
using namespace albert;
using chrono::duration_cast;
using chrono::milliseconds;
using chrono::system_clock;

PluginRegistry::PluginRegistry(ExtensionRegistry &er) : ExtensionWatcher<PluginProvider>(er)
{
}

vector<const PluginLoader*> PluginRegistry::plugins() const
{
    vector<const PluginLoader*> plugins;
    for (const auto &[id, loader] : plugins_)
        plugins.emplace_back(loader);
    return plugins;
}

bool PluginRegistry::isEnabled(const QString &id) const
{
    return QSettings(qApp->applicationName()).value(QString("%1/enabled").arg(id), false).toBool();
}

void PluginRegistry::enable(const QString &id, bool enable)
{
    if (plugins_.contains(id)){
        if (isEnabled(id) != enable){
            QSettings(qApp->applicationName()).setValue(QString("%1/enabled").arg(id), enable);
            load(id, enable);
        }
    } else
        WARN << "En-/Disabled nonexistent id:" << id;
}

void PluginRegistry::load(const QString &id, bool load)
{
    try {
        auto *loader = plugins_.at(id);
        switch (loader->state()) {
        case PluginState::Invalid:
            WARN << "Tried to" << (load ? "load" : "unload") << "invalid plugin" << id;
            break;
        case PluginState::Loaded:
            if (!load) {
                DEBG << "Unloading plugin" << loader->metaData().id;
                auto start = system_clock::now();
                loader->unload();
                auto duration = duration_cast<milliseconds>(system_clock::now()-start).count();
                if (loader->state() == PluginState::Unloaded){
                    DEBG << QString("[%1ms] Successfully unloaded '%2'")
                                .arg(duration).arg(loader->metaData().id);
                    emit pluginStateChanged(id);
                } else
                    WARN << QString("[%1ms] Failed unloading '%2': %3")
                                .arg(duration).arg(loader->metaData().id, loader->stateInfo());
            } else
                WARN << "Plugin is already loaded:" << id ;
            break;
        case PluginState::Unloaded:
            if (load){
                INFO << "Loading plugin" << loader->metaData().id;
                auto start = system_clock::now();
                loader->load();
                auto duration = duration_cast<milliseconds>(system_clock::now()-start).count();
                if (loader->state() == PluginState::Loaded){
                    DEBG << QString("[%1ms] Successfully loaded '%2'")
                                .arg(duration).arg(loader->metaData().id);
                    emit pluginStateChanged(id);
                } else
                    WARN << QString("[%1ms] Failed loading '%2': %3")
                                .arg(duration).arg(loader->metaData().id, loader->stateInfo());
            } else
                WARN << "Plugin is already unloaded:" << id ;
            break;
        }
    } catch (const exception &e) {
        WARN << QString("Error while (un-)loading plugin '%1': %2").arg(id, e.what());
    }
}

void PluginRegistry::onAdd(PluginProvider *pp)
{
    for (auto *loader : pp->plugins()){
        if (loader->state() == PluginState::Invalid)
            continue;
        const auto &id = loader->metaData().id;
        if (const auto &[it, success] = plugins_.emplace(id, loader); success){
            if (loader->metaData().user && isEnabled(id))
                load(id);
        } else
            INFO << "Plugin" << id << "shadowed:" << loader->path;
    }
    emit pluginsChanged();
}

void PluginRegistry::onRem(PluginProvider *pp)
{
    for (auto it = plugins_.begin(); it != plugins_.end();){
        const auto &[id, loader] = *it;
        if (loader->provider() == pp){
            if (loader->state() == PluginState::Loaded)
                loader->unload();
            it = plugins_.erase(it);
        } else ++it;
    }
    emit pluginsChanged();
}
