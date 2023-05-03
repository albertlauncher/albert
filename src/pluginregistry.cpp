// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extensions/pluginprovider.h"
#include "albert/logging.h"
#include "albert/util/standarditem.h"
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

QString PluginRegistry::id() const { return "pluginregistry"; }

QString PluginRegistry::name() const { return "Plugins"; }

QString PluginRegistry::description() const { return "Manage plugins"; }

QString PluginRegistry::defaultTrigger() const { return QStringLiteral("plug "); }

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

void PluginRegistry::handleTriggerQuery(TriggerQuery &query) const
{
    for (const auto &[id, loader] : plugins_){  // these should all be valid

        if (!id.startsWith(query.string(), Qt::CaseInsensitive))
            continue;

        Actions actions;
        QString info;

        auto mutable_this = const_cast<PluginRegistry*>(this);

        if (loader->metaData().user){
            if (isEnabled(id))
                actions.emplace_back(
                    "disable", "Disable plugin",
                    [mutable_this, id=id]() { mutable_this->enable(id, false); }
                );
            else
                actions.emplace_back(
                    "enable", "Enable plugin",
                    [mutable_this, id=id](){ mutable_this->enable(id); }
                    );

            if (loader->state() == PluginState::Loaded){
                actions.emplace_back(
                    "unload", "Unload plugin",
                    [mutable_this, id=id](){ mutable_this->load(id, false); }
                );
                actions.emplace_back(
                    "reload", "Reload plugin",
                    [mutable_this, id=id](){ mutable_this->load(id, false); mutable_this->load(id, true); }
                );
            }
            else  // by contract only unloaded
                actions.emplace_back(
                    "load", "Load plugin",
                    [mutable_this, id=id](){ mutable_this->load(id); }
                );

            QString enabled = isEnabled(id) ? "Enabled" : "Disabled";
            QString state;
            if (loader->state() == PluginState::Loaded)
                state = "Loaded";
            else if (loader->stateInfo().isEmpty())
                state = "Unloaded";
            else
                state = QString("Error: %1").arg(loader->stateInfo());
            info = QString("[%1, %2] ").arg(enabled, state);
        }
        info.append(loader->metaData().description);

        query.add(
            StandardItem::make(
                id,
                loader->metaData().name,
                info,
                {":app_icon"},
                actions
            )
        );
    }

}

