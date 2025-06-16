// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <QObject>
#include <QString>
#include <map>
#include <set>
namespace albert {
class Extension;
class ExtensionRegistry;
class PluginInstance;
class PluginLoader;
class PluginMetadata;
class PluginProvider;
}


class Plugin
{
public:
    albert::PluginLoader &loader;
    const QString &id;  // convenience reference to loader.metadata.id
    const albert::PluginMetadata &metadata;  // convenience reference to loader.metadata
    enum class State {
        Unloaded,
        Loading,
        Loaded
    } state;
    QString state_info;
    std::vector<albert::Extension*> registered_extensions;
    const albert::PluginProvider &provider;
    bool enabled;
};


class PluginRegistry : public QObject
{
    Q_OBJECT

public:

    PluginRegistry(albert::ExtensionRegistry&, bool autoload_enabled_plugins);
    ~PluginRegistry();

    /// Get map of all registered plugins
    const std::map<QString, Plugin> &plugins() const;

    /// @throws std::out_of_range if `id` is not in \ref plugins().
    void setEnabledWithUserConfirmation(const QString id, bool enable);

    /// Enable/Disable a plugin and its transitive dependencies/dependees.
    /// @throws std::out_of_range if `id` is not in \ref plugins().
    void setEnabled(const QString &id, bool enable);

    /// (Un)Load a plugin and its transitive dependencies/dependees.
    /// @throws std::out_of_range if `id` does not exist.
    void setLoaded(const QString &id, bool load);

    std::set<const Plugin*> dependencies(const Plugin*) const;
    std::set<const Plugin*> dependees(const Plugin*) const;
    std::set<const Plugin*> dependencyClosure(const std::set<const Plugin*>&) const;
    std::set<const Plugin*> dependeeClosure(const std::set<const Plugin*>&) const;

private:

    void load(std::set<const Plugin*>);
    void unload(std::set<const Plugin *>);

    void onRegistered(albert::PluginProvider*);
    void onDeregistered(albert::PluginProvider*);

    albert::ExtensionRegistry &extension_registry_;
    std::set<albert::PluginProvider*> plugin_providers_;
    std::map<QString, Plugin> plugins_;
    bool load_enabled_;

    std::set<const Plugin*> loading_plugins_;
    std::map<const Plugin*, std::set<const Plugin*>> loading_graph_;
    void onPluginLoaderFinished(Plugin &p, const QString &info);

signals:

    void pluginsChanged();
    void pluginEnabledChanged(const QString &id);
    void pluginStateChanged(const QString &id);

};
