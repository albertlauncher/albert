// Copyright (c) 2023-2025 Manuel Schneider

#include "albert.h"
#include "extensionregistry.h"
#include "logging.h"
#include "messagebox.h"
#include "plugininstance.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginprovider.h"
#include "pluginregistry.h"
#include "topologicalsort.hpp"
#include <QCoreApplication>
#include <QSettings>
#include <QTimer>
#include <ranges>
using enum Plugin::State;
using enum albert::PluginMetadata::LoadType;
using namespace albert;
using namespace std;

PluginRegistry::PluginRegistry(ExtensionRegistry &reg, bool autoload_enabled_plugins) :
    extension_registry_(reg),
    load_enabled_(autoload_enabled_plugins)
{
    connect(&extension_registry_, &ExtensionRegistry::added,
            this, [this](Extension *e)
            { if (auto p = dynamic_cast<PluginProvider*>(e)) onRegistered(p); });

    connect(&extension_registry_, &ExtensionRegistry::removed,
            this, [this](Extension *e)
            { if (auto p = dynamic_cast<PluginProvider*>(e)) onDeregistered(p); });
}

PluginRegistry::~PluginRegistry()
{
    if (!plugin_providers_.empty())
        WARN << "PluginRegistry destroyed with active plugin providers";

    if (!plugins_.empty())
        WARN << "PluginRegistry destroyed with active plugins";

    if (!loading_plugins_.empty())
    {
        QEventLoop loop;
        connect(this, &PluginRegistry::pluginStateChanged, this, [&] {
            if (loading_plugins_.empty())
                loop.quit();
        });
        loop.exec();
    }
}

const map<QString, Plugin> &PluginRegistry::plugins() const { return plugins_; }

void PluginRegistry::setEnabledWithUserConfirmation(const QString id, bool enable)
{
    auto &plugin = plugins_.at(id);

    if (plugin.enabled == enable)
        return;

    auto v = (enable ? dependencyClosure({&plugin}) : dependeeClosure({&plugin}))
             | views::filter([&](auto p) { return p->metadata.load_type == User
                                                  && p->id != id
                                                  && p->enabled != enable;})
             | views::transform([](auto p) { return p->metadata.name; });
    QStringList names(v.begin(), v.end());  // ranges::to

    if (!names.empty())
    {
        auto text = (enable ? tr("Enabling '%1' will also enable the following plugins")
                            : tr("Disabling '%1' will also disable the following plugins"))
                        .arg(plugin.metadata.name);
        text.append(QString(":\n\n").append(names.join("\n")));

        if (!util::question(text))
            return;
    }

    setEnabled(id, enable);
}

void PluginRegistry::setEnabled(const QString &id, bool enable)
{
    auto &plugin = plugins_.at(id);

    if (plugin.enabled == enable)
        return;

    for (auto p : enable ? dependencyClosure({&plugin}) : dependeeClosure({&plugin}))
        if (p->metadata.load_type == User && p->enabled != enable)
        {
            const_cast<Plugin*>(p)->enabled = enable;  // safe, original is not const
            settings()->setValue(QString("%1/enabled").arg(p->id), enable);
            emit pluginEnabledChanged(p->id);
        }

    // Clear state info on disabling an unloaded plugin.
    if (!enable && plugin.state == Unloaded)
        plugin.state_info.clear();

    setLoaded(id, enable);
}

void PluginRegistry::setLoaded(const QString &id, bool loaded)
{
    if (!loading_plugins_.empty())
        util::warning(QStringLiteral("%1: %2")
                          .arg(loaded ? tr("Failed to load plugin")
                                      : tr("Failed to unload plugin"),
                               tr("Other plugins are currently being loaded.")));

    else if (loaded)
        load({&plugins_.at(id)});
    else
        unload({&plugins_.at(id)});
}

std::set<const Plugin *> PluginRegistry::dependencies(const Plugin *p) const
{
    auto v = p->loader.metadata().plugin_dependencies
             | views::transform([this](const QString &id){ return &plugins_.at(id); });
    return {v.begin(), v.end()};  // ranges::to
}

std::set<const Plugin *> PluginRegistry::dependees(const Plugin *p) const
{
    auto v =  plugins_
             | views::transform([](auto &pair){ return &pair.second; })
             | views::filter([p](const Plugin *o){
                   return ranges::any_of(o->metadata.plugin_dependencies,
                                         [p](auto &id){ return id == p->id;});
               });
    return {v.begin(), v.end()};  // ranges::to
}

set<const Plugin *> PluginRegistry::dependencyClosure(const std::set<const Plugin*> &plugins) const
{
    auto D = plugins;
    for (auto p : plugins)
        for (auto d : dependencies(p))
            D.merge(dependencyClosure({d}));
    return D;
}

set<const Plugin *> PluginRegistry::dependeeClosure(const std::set<const Plugin*> &plugins) const
{
    auto D = plugins;
    for (auto p : plugins)
        for (auto d : dependees(p))
            D.merge(dependeeClosure({d}));
    return D;
}

void PluginRegistry::onRegistered(PluginProvider *pp)
{
    // Register plugin provider

    const auto &[_, pp_reg_success] = plugin_providers_.insert(pp);
    if (!pp_reg_success)
        qFatal("Plugin provider registered twice.");

    // Make unique plugins

    map<QString, PluginLoader*> unique_loaders;
    for (auto loader : pp->plugins())
        if (const auto &[it, succ] = unique_loaders.emplace(loader->metadata().id,
                                                            loader);
            !succ)
            INFO << QString("Plugin '%1' at '%2' shadowed by '%3'")
                        .arg(it->first, loader->path(), it->second->path());

    // Topo sort once to filter by valid dependencies

    map<QString, set<QString>> dependency_graph;
    for (auto &[id, loader] : unique_loaders)
        dependency_graph.emplace(id, set<QString>{begin(loader->metadata().plugin_dependencies),
                                                  end(loader->metadata().plugin_dependencies)});  // ranges::to

    if (auto topo_result = topologicalSort(dependency_graph);
        !topo_result.error_set.empty())
    {
        for (const auto &[id, deps] : topo_result.error_set)
        {
            WARN << "Skipping plugin" << id << "because of missing or cyclic dependencies.";
            unique_loaders.erase(id);
        }

        WARN << "Error set:";
        for (const auto &[id, deps] : topo_result.error_set)
            for (const auto &dep : deps)
                WARN << id << "→" << dep;
    }

    // Finally register the valid plugins

    for (auto &[id, loader] : unique_loaders)
    {
        Plugin p{
            .loader = *loader,
            .id = loader->metadata().id,
            .metadata = loader->metadata(),
            .state = Unloaded,
            .state_info = {},
            .registered_extensions={},
            .provider = *pp,
            .enabled = settings()->value(QString("%1/enabled").arg(id), false).toBool()
        };

        if (const auto &[it, success] = plugins_.emplace(id, p);
            success)
            connect(loader, &PluginLoader::finished,
                    this, [this, &p=it->second](QString info){ onPluginLoaderFinished(p, info); });
        else
            CRIT << "Logic error: Plugin already exists" << it->first;
    }

    emit pluginsChanged();

    if (load_enabled_)
    {
        auto v = plugins_
                 | views::transform([](auto &p) { return &p.second; })
                 | views::filter([pp](auto p) { return &p->provider == pp
                                                       && p->metadata.load_type == User
                                                       && p->enabled; });
        load({v.begin(), v.end()});  // ranges::to
    }
}

void PluginRegistry::onDeregistered(PluginProvider *pp)
{
    const auto filter = [pp](const auto& it){ return &it.second.provider == pp; };

    // Unload plugins of this provider
    auto v = plugins_ | views::filter(filter) | views::transform([](auto &p){ return &p.second; });
    unload({v.begin(), v.end()});  // ranges::to

    // Remove plugins of this provider
    erase_if(plugins_, filter);
    emit pluginsChanged();

    // Remove provider
    if (!plugin_providers_.erase(pp))
        qFatal("Plugin provider was not registered onRem.");
}

void PluginRegistry::load(set<const Plugin*> plugins)
{
    // Make dependency graph
    map<const Plugin*, set<const Plugin*>> graph;
    for (auto p : dependencyClosure(plugins))
        graph.emplace(p, dependencies(p));

    // Remove loaded plugins from graph
    erase_if(graph, [](auto &p){ return p.first->state == Loaded; });
    for (auto &[p, deps] : graph)
        erase_if(deps, [](auto p_){ return p_->state == Loaded; });

    if (graph.empty())
        return;

    loading_graph_.merge(::move(graph));

    // Load initial set without dependencies
    for (auto it = begin(loading_graph_); it != end(loading_graph_);)
        if (it->second.empty())
        {
            Plugin &p = *const_cast<Plugin*>(it->first);

            loading_plugins_.insert(&p);
            it = loading_graph_.erase(it);

            setPluginState(p, Loading);
            p.loader.load();  // may be async
        }
        else
            ++it;
}

void PluginRegistry::unload(set<const Plugin*> plugins)
{
    // Make dependee graph
    map<const Plugin*, set<const Plugin*>> graph;
    for (auto p : dependeeClosure(plugins))
        graph.emplace(p, dependees(p));

    // Remove unloaded plugins from graph
    erase_if(graph, [](auto &p){ return p.first->state == Unloaded; });
    for (auto &[p, deps] : graph)
        erase_if(deps, [](auto p_){ return p_->state == Unloaded; });

    if (graph.empty())
        return;

    auto topo = topologicalSort(graph);

    for (const Plugin *cp : topo.sorted)
    {
        Plugin &p = *const_cast<Plugin*>(cp);
        for (auto *e : p.registered_extensions)
            extension_registry_.deregisterExtension(e);

        p.loader.unload();
        setPluginState(p, Unloaded);
    }
}

void PluginRegistry::onPluginLoaderFinished(Plugin &p, const QString &info)
{
    setPluginState(p, p.loader.instance() ? Loaded : Unloaded, info);

    // remove from loading plugins
    loading_plugins_.erase(&p);

    // remove dependecies in load graph
    for (auto it = begin(loading_graph_); it != end(loading_graph_);)
        if (it->second.erase(&p) && it->second.empty())
        {
            Plugin &p_ = *const_cast<Plugin*>(it->first);

            loading_plugins_.insert(&p_);
            it = loading_graph_.erase(it);

            setPluginState(p_, Loading);
            p_.loader.load();  // may be async
        }
        else
            ++it;

    if (p.loader.instance())
    {
        try {
            p.registered_extensions = p.loader.instance()->extensions();
            for (auto *e : p.registered_extensions)
                extension_registry_.registerExtension(e);
        } catch (const exception &e) {
            CRIT << p.id << "Exception in PluginInstance::extensions()" << e.what();
        } catch (...) {
            CRIT << p.id << "Unknown exception in PluginInstance::extensions()";
        }
    }
}

void PluginRegistry::setPluginState(Plugin &plugin, Plugin::State state, const QString info)
{
    plugin.state = state;
    plugin.state_info = info;
    switch (state)
    {
    case Unloaded:
        DEBG << "Plugin" << plugin.id << "unloaded." << info;
        break;
    case Loading:
        DEBG << "Plugin" << plugin.id << "loading.";
        break;
    case Loaded:
        DEBG << "Plugin" << plugin.id << "loaded." << info;
        break;
    }
    emit pluginStateChanged(plugin.id);
}
