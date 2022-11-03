// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/pluginprovider.h"
#include <QIcon>
#include <QString>
#include <map>
namespace albert{
class Plugin;
class Frontend;
class ExtensionRegistry;
}

class PluginProvider final : public albert::PluginProvider
{
public:
    explicit PluginProvider(albert::ExtensionRegistry &);
    ~PluginProvider() override;

    void findPlugins(const QStringList &paths);
    void loadPlugins();
    void unloadPlugins();

    albert::Frontend * frontend;
    const std::vector<albert::PluginSpec> &frontends();
    void setFrontend(uint);

    // Interfaces
    QString id() const override;
    QIcon icon() const override;
    const std::map<QString, albert::PluginSpec> &plugins() const override;
    bool isEnabled(const QString &id) const override;
    void setEnabled(const QString &id, bool enabled) override;

private:

    albert::PluginSpec parsePluginMetadata(const QString& path);
    albert::Plugin *loadPlugin(const QString &id);
    void unloadPlugin(const QString &id);

    void loadFrontend();
    void loadUserPlugins();

    std::vector<albert::PluginSpec> frontends_;
    std::map<QString,albert::PluginSpec> specs;
    albert::ExtensionRegistry &registry;


//    // ALL plugins
//    std::vector<std::unique_ptr<NativePluginSpec>> plugins_;
//    std::map<QString,NativePluginSpec*> plugin_index;
//    // VALID plugins
//    std::map<NativePluginSpec*,std::set<NativePluginSpec*>> transitive_dependencies;
//    std::map<NativePluginSpec*,std::set<NativePluginSpec*>> transitive_dependees;
};

