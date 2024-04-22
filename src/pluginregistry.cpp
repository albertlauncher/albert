// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extension/pluginprovider/pluginprovider.h"
#include "albert/logging.h"
#include "pluginregistry.h"
#include <QApplication>
#include <QMessageBox>
using namespace albert;
using namespace std;

PluginRegistry::PluginRegistry(ExtensionRegistry &registry, bool load_enabled):
    ExtensionWatcher<PluginProvider>(&registry),
    extension_registry_(registry),
    load_enabled_(load_enabled){}

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
            QStringList ids;
            for (const auto &d : to_enable)
                ids << d->id();

            auto text = tr("Enabling '%1' will also enable the following plugins:").arg(plugin.id());
            text.append(QString("\n\n%1").arg(ids.join("\n")));

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
            QStringList ids;
            for (const auto &d : to_disable)
                ids << d->id();

            auto text = tr("Disabling '%1' will also disable the following plugins:").arg(plugin.id());
            text.append(QString("\n\n%1").arg(ids.join("\n")));

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
            if (auto err = p->load(extension_registry_); !err.isEmpty())
            {
                WARN << QString("Failed loading plugin '%1': %2").arg(p->id(), err);
                errors << QString("%1 (%2):\n%3").arg(p->metaData().name, p->id(), err);
            }

        if (!errors.isEmpty())
            QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                                 QString("%1:\n\n%2")
                                     .arg(tr("Failed loading plugins", nullptr, errors.size()),
                                          errors.join("\n")));
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
            if (auto err = p->unload(extension_registry_); !err.isEmpty())
            {
                WARN << QString("Failed unloading plugin '%1': %2").arg(p->id(), err);
                errors << QString("%1 (%2):\n%3").arg(p->metaData().name, p->id(), err);
            }

        if (!errors.isEmpty())
            QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                                 QString("%1:\n\n%2")
                                     .arg(tr("Failed unloading plugins", nullptr, errors.size()),
                                          errors.join("\n")));
    }
    catch (const std::out_of_range &) {
        WARN << "Plugin does not exist:" << id;
    }
}

void PluginRegistry::onAdd(PluginProvider *plugin_provider)
{
    // Register plugin provider
    const auto &[_, pp_reg_success] = plugin_providers_.insert(plugin_provider);
    if (!pp_reg_success)
        qFatal("Plugin provider registered twice.");


    // Register plugins enforcing unique ids
    std::set<Plugin*> registered_plugins;
    for (auto &plugin_loader : plugin_provider->plugins())
    {
        if (const auto &[it, pl_reg_success]
            = registered_plugins_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(plugin_loader->metaData().id),
                                          std::forward_as_tuple(plugin_provider, plugin_loader));
            pl_reg_success)
            registered_plugins.insert(&it->second);
        else
            INFO << QString("Plugin '%1' at '%2' shadowed by '%3'")
                        .arg(it->first, plugin_loader->path(), it->second.loader->path());
    }

    // Topological sort/load order (Kahn's algorithm), also populate dependencies and dependees
    map<QString, set<QString>> dependencies;
    for (const auto &plugin : registered_plugins)
        dependencies.emplace(std::piecewise_construct,
                             std::forward_as_tuple(plugin->id()),
                             std::forward_as_tuple(begin(plugin->loader->metaData().plugin_dependencies),
                                                   end(plugin->loader->metaData().plugin_dependencies)));

    std::vector<QString> load_order;
    std::vector<QString> no_dep_plugins;

    // init no_dep_plugins. the "start-node-set".
    for (auto it = dependencies.begin(); it != dependencies.end();)
    {
        const auto &[id, deps] = *it;
        if (deps.empty())
        {
            no_dep_plugins.push_back(id);
            it = dependencies.erase(it);
        }
        else
            ++it;
    }

    while (!no_dep_plugins.empty())
    {
        const auto &dependency_id = no_dep_plugins.back();
        no_dep_plugins.pop_back();

        load_order.push_back(dependency_id);

        for (auto it = dependencies.begin(); it != dependencies.end();)
        {
            auto &[dependee_id, dependee_dependecies] = *it;

            if (auto ddit = dependee_dependecies.find(dependency_id); ddit != dependee_dependecies.end())
            {
                registered_plugins_.at(dependee_id).dependencies_.insert(&registered_plugins_.at(dependency_id));
                registered_plugins_.at(dependency_id).dependees_.insert(&registered_plugins_.at(dependee_id));
                dependee_dependecies.erase(ddit);

                if (dependee_dependecies.empty())
                {
                    no_dep_plugins.push_back(dependency_id);
                    it = dependencies.erase(it);
                }
                else
                    ++it;
            }
            else
                ++it;
        }
    }

    // Check for cycles
    if (!dependencies.empty())
    {
        QString msg = tr("Cyclic or missing dependencies detected:");
        for (const auto &[id, deps] : dependencies)
        {
            msg += QString("\n%1: %2").arg(id, QStringList(deps.cbegin(), deps.cend()).join(", "));
            registered_plugins_.erase(id);
        }

        QMessageBox::critical(nullptr, qApp->applicationDisplayName(), msg);
    }

    // Set plugin load order fields
    for (uint i = 0; i < load_order.size(); ++i)
        registered_plugins_.at(load_order[i]).load_order = i;

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
        if (auto err = p->load(extension_registry_); !err.isEmpty())
        {
            WARN << QString("Failed loading plugin '%1': %2").arg(p->id(), err);
            errors << QString("%1 (%2):\n%3").arg(p->metaData().name, p->id(), err);
        }

    if (!errors.isEmpty())
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                             QString("%1:\n\n%2")
                                 .arg(tr("Failed loading plugins", nullptr, errors.size()),
                                      errors.join("\n")));
}

void PluginRegistry::onRem(PluginProvider *plugin_provider)
{
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
        if (auto err = p->unload(extension_registry_); !err.isEmpty())
        {
            WARN << QString("Failed unloading plugin '%1': %2").arg(p->id(), err);
            errors << QString("%1 (%2):\n%3").arg(p->metaData().name, p->id(), err);
        }

    if (!errors.isEmpty())
        QMessageBox::warning(nullptr, qApp->applicationDisplayName(),
                             QString("%1:\n\n%2")
                                 .arg(tr("Failed unloading plugins", nullptr, errors.size()),
                                      errors.join("\n")));

    // Remove registerd plugins of this provider
    erase_if(registered_plugins_, [=](const auto& it){ return it.second.provider == plugin_provider; });

    // Remove provider
    plugin_providers_.erase(plugin_provider);

    emit pluginsChanged();
}
