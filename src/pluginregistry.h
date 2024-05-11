// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/util/extensionwatcher.h"
#include "plugin.h"
#include <QObject>
#include <QString>
#include <map>
#include <set>
namespace albert {
class ExtensionRegistry;
class PluginLoader;
class PluginInstance;
class PluginProvider;
}

class PluginRegistry : public QObject, public albert::ExtensionWatcher<albert::PluginProvider>
{
    Q_OBJECT

public:

    PluginRegistry(albert::ExtensionRegistry &extension_registry, bool load_enabled = true);
    ~PluginRegistry();

    const std::map<QString, Plugin> &plugins();

    /// Enables and loads a plugin and its dependencies
    /// Asks the user for confirmation. Shows errors in message boxes.
    void enable(const QString &id);

    /// Disables and unloads a plugin and all plugins that depend on it
    /// Asks the user for confirmation. Shows errors in message boxes.
    void disable(const QString &id);

    /// Loads a plugin and all its dependencies
    /// Asks the user for confirmation. Shows errors in message boxes.
    void load(const QString &id);

    /// Unloads a plugin and all plugins that depend on it
    /// Asks the user for confirmation. Shows errors in message boxes.
    void unload(const QString &id);

    static struct StaticDI {
        albert::PluginLoader * loader;
        albert::ExtensionRegistry *registry;
    } staticDI;

private:

    void onAdd(albert::PluginProvider*) override;
    void onRem(albert::PluginProvider*) override;

    albert::ExtensionRegistry &extension_registry_;
    std::set<albert::PluginProvider*> plugin_providers_;
    std::map<QString, Plugin> registered_plugins_;
    bool load_enabled_;

signals:

    void pluginsChanged();

};
