// Copyright (c) 2023-2024 Manuel Schneider

#include "extensionregistry.h"
#include "logging.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginprovider.h"
#include "pluginregistry.h"
#include "topologicalsort.hpp"
#include <QApplication>
#include <QMessageBox>
using namespace albert;
using namespace std;

PluginRegistry::StaticDI PluginRegistry::staticDI
{
    .loader = nullptr,
    .registry = nullptr
};

PluginRegistry::PluginRegistry(ExtensionRegistry &registry, bool load_enabled):
    extension_registry_(registry),
    load_enabled_(load_enabled)
{
    // nope only one
    if (staticDI.registry)
        qFatal("nope only one PluginRegistry");
    staticDI.registry = &registry;

    connect(&extension_registry_, &ExtensionRegistry::added,
            this, &PluginRegistry::onRegistered);

    connect(&extension_registry_, &ExtensionRegistry::removed,
            this, &PluginRegistry::onDeregistered);
}

PluginRegistry::~PluginRegistry()
{
    if (!plugin_providers_.empty())
        WARN << "PluginRegistry destroyed with active plugin providers";

    if (!registered_plugins_.empty())
        WARN << "PluginRegistry destroyed with active plugins";
}

const map<QString, Plugin> &PluginRegistry::plugins() { return registered_plugins_; }

void PluginRegistry::enable(const QString &id)
{
    try {
        auto &plugin = registered_plugins_.at(id);

        if (plugin.isEnabled())
            return;

        set<Plugin*> dependencies = plugin.transitiveDependencies();

        set<Plugin*> to_enable;
        for (const auto &d : dependencies)
            if (!d->isEnabled())
                to_enable.insert(d);

        if (!to_enable.empty())
        {
            QStringList names;
            for (const auto &d : to_enable)
                names << d->metaData().name;

            auto text = tr("Enabling '%1' will also enable the following plugins:").arg(plugin.id());
            text.append(QString("\n\n%1").arg(names.join("\n")));

            auto btn = QMessageBox::question(nullptr, qApp->applicationDisplayName(),
                                             text, QMessageBox::Ok|QMessageBox::Cancel);
            if (btn == QMessageBox::Cancel)
                return;

            for (const auto &d : to_enable)
                d->setEnabled(true);
        }

        plugin.setEnabled(true);

        load(id);
    }
    catch (const std::out_of_range &) {
        WARN << "Plugin does not exist:" << id;
    }
}

void PluginRegistry::disable(const QString &id)
{
    try {
        auto &plugin = registered_plugins_.at(id);

        if (!plugin.isEnabled())
            return;

        set<Plugin*> dependees = plugin.transitiveDependees();

        set<Plugin*> to_disable;
        for (const auto &d : dependees)
            if (d->isEnabled())
                to_disable.insert(d);

        if (!to_disable.empty())
        {
            QStringList names;
            for (const auto &d : to_disable)
                names << d->metaData().name;

            auto text = tr("Disabling '%1' will also disable the following plugins:").arg(plugin.id());
            text.append(QString("\n\n%1").arg(names.join("\n")));

            auto btn = QMessageBox::question(nullptr, qApp->applicationDisplayName(),
                                             text, QMessageBox::Ok|QMessageBox::Cancel);
            if (btn == QMessageBox::Cancel)
                return;

            for (const auto &d : to_disable)
                d->setEnabled(false);
        }

        plugin.setEnabled(false);

        unload(id);
    }
    catch (const std::out_of_range &) {
        WARN << "Plugin does not exist:" << id;
    }
}

void PluginRegistry::load(const QString &id)
{
    try {
        auto &plugin = registered_plugins_.at(id);

        auto s = plugin.transitiveDependencies();
        s.insert(&plugin);

        vector<Plugin*> v{s.cbegin(), s.cend()};
        ::sort(v.begin(), v.end(), [](auto *l, auto *r){ return l->load_order < r->load_order; });

        QStringList errors;
        for (auto *p : v)
            if (p->state() != Plugin::State::Loaded)
                if (auto err = p->load(); !err.isEmpty())
                {
                    WARN << QString("Failed loading plugin '%1': %2").arg(p->id(), err);
                    errors << p->metaData().name;
                }

        if (!errors.isEmpty())
            QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                                 QString("%1:\n\n%2\n\n%3")
                                     .arg(tr("Failed loading plugins", nullptr, errors.size()),
                                          errors.join("\n"),
                                          tr("Check the log for more information.")));
    }
    catch (const std::out_of_range &) {
        WARN << "Plugin does not exist:" << id;
    }
}

void PluginRegistry::unload(const QString &id)
{
    try {
        auto &plugin = registered_plugins_.at(id);

        auto s = plugin.transitiveDependees();
        s.insert(&plugin);

        vector<Plugin*> v{s.cbegin(), s.cend()};
        ::sort(v.begin(), v.end(), [](auto *l, auto *r){ return l->load_order > r->load_order; });

        QStringList errors;
        for (auto *p : v)
            if (auto err = p->unload(); !err.isEmpty())
            {
                WARN << QString("Failed unloading plugin '%1': %2").arg(p->id(), err);
                errors << p->metaData().name;
            }

        if (!errors.isEmpty())
            QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                                 QString("%1:\n\n%2\n\n%3")
                                     .arg(tr("Failed unloading plugins", nullptr, errors.size()),
                                          errors.join("\n"),
                                          tr("Check the log for more information.")));
    }
    catch (const std::out_of_range &) {
        WARN << "Plugin does not exist:" << id;
    }
}

void PluginRegistry::onRegistered(Extension *e)
{
    auto *plugin_provider = dynamic_cast<PluginProvider*>(e);
    if (!plugin_provider)
        return;

            // Register plugin provider
    const auto &[_, pp_reg_success] = plugin_providers_.insert(plugin_provider);
    if (!pp_reg_success)
        qFatal("Plugin provider registered twice.");

    // Make the plugins unique by id

    std::map<QString, PluginLoader*> unique_loaders;
    for (auto &loader : plugin_provider->plugins())
        if (const auto &[it, succ] = unique_loaders.emplace(loader->metaData().id, loader); !succ)
            INFO << QString("Plugin '%1' at '%2' shadowed by '%3'")
                        .arg(it->first, loader->path(), it->second->path());

    // Filter malformed dependencies and get the load order

    map<QString, set<QString>> dependency_graph;
    for (const auto&[id, loader] : unique_loaders)
        dependency_graph.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(id),
                                 std::forward_as_tuple(begin(loader->metaData().plugin_dependencies),
                                                       end(loader->metaData().plugin_dependencies)));

    auto topo = topologicalSort(dependency_graph);

    if (!topo.error_set.empty())
    {
        auto msg = tr("Cyclic or missing dependencies detected:");
        for (const auto &[id, deps] : topo.error_set)
        {
            msg += QString("\n\n%1: %2").arg(id,
                                             QStringList(dependency_graph.at(id).cbegin(),
                                                         dependency_graph.at(id).cend()).join(", "));
            unique_loaders.erase(id);
        }
        WARN << msg;
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
    }

    // Register plugins and set load order, dependencies and dependees

    int load_order{0};
    for (const auto &id : topo.sorted)
    {
        const auto &[it, succ] = registered_plugins_.emplace(std::piecewise_construct,
                                                             std::forward_as_tuple(id),
                                                             std::forward_as_tuple(plugin_provider, unique_loaders.at(id)));
        if (!succ)
            qFatal("Duplicate plugin id registered: %s", qPrintable(it->first));

        auto &plugin = it->second;
        plugin.load_order = load_order++;
        for (const auto &dependency_id : plugin.loader->metaData().plugin_dependencies)
        {
            auto &dep = registered_plugins_.at(dependency_id);
            plugin.dependencies_.insert(&dep);
            dep.dependees_.insert(&plugin);
        }

        // Signal mappers
        connect(&plugin, &Plugin::enabledChanged,
                this, [this, &plugin] { emit enabledChanged(plugin.id()); });

        connect(&plugin, &Plugin::stateChanged,
                this, [this, &plugin] { emit stateChanged(plugin.id()); });
    }

    emit pluginsChanged();

    if (!load_enabled_)
        return;

    // Load enabled user plugins of this provider
    vector<Plugin*> plugins_to_load;
    for (auto &[id, plugin] : registered_plugins_)
        if (plugin.provider == plugin_provider && plugin.isUser() && plugin.isEnabled())
            plugins_to_load.push_back(&plugin);

    // Sort by load order
    ::sort(plugins_to_load.begin(), plugins_to_load.end(),
           [](const auto *l, const auto *r){ return l->load_order < r->load_order; });

    // Load enabled plugins
    QStringList errors;
    for (auto *p : plugins_to_load)
        if (auto err = p->load(); !err.isEmpty())
        {
            WARN << QString("Failed loading plugin '%1': %2").arg(p->id(), err);
            errors << p->metaData().name;
        }

    if (!errors.isEmpty())
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                             QString("%1:\n\n%2\n\n%3")
                                 .arg(tr("Failed loading plugins", nullptr, errors.size()),
                                      errors.join("\n"),
                                      tr("Check the log for more information.")));
}

void PluginRegistry::onDeregistered(Extension *e)
{
    auto *plugin_provider = dynamic_cast<PluginProvider*>(e);
    if (!plugin_provider)
        return;

    // unload all plugins of this provider
    vector<Plugin*> plugins_to_unload;
    for (auto &[id, plugin] : registered_plugins_)
        if (plugin.provider == plugin_provider && plugin.isUser() && plugin.state() == Plugin::State::Loaded)
            plugins_to_unload.push_back(&plugin);

    // Sort by load order (reversed to unload)
    ::sort(plugins_to_unload.begin(), plugins_to_unload.end(),
           [](const auto *l, const auto *r){ return l->load_order > r->load_order; });

    // Unload plugins
    QStringList errors;
    for (auto *p : plugins_to_unload)
        if (auto err = p->unload(); !err.isEmpty())
        {
            WARN << QString("Failed unloading plugin '%1': %2").arg(p->id(), err);
            errors << p->metaData().name;
        }

    if (!errors.isEmpty())
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                             QString("%1:\n\n%2\n\n%3")
                                 .arg(tr("Failed unloading plugins", nullptr, errors.size()),
                                      errors.join("\n"),
                                      tr("Check the log for more information.")));

    // Remove registerd plugins of this provider
    erase_if(registered_plugins_, [=](const auto& it){ return it.second.provider == plugin_provider; });

    // Remove provider
    plugin_providers_.erase(plugin_provider);

    emit pluginsChanged();
}
