// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/pluginprovider.h"
#include <vector>
namespace albert {
class Frontend;
class Plugin;
class ExtensionRegistry;
}

class NativePluginProvider final : public albert::PluginProvider
{
public:
    explicit NativePluginProvider(albert::ExtensionRegistry&, const QStringList& additional_paths);
    ~NativePluginProvider() override;

    void loadFrontend();
    void loadEnabledPlugins();

    albert::Frontend *frontend();
    const std::vector<albert::PluginSpec*> &frontendPlugins();
    void setFrontend(uint);

    // albert::Extension
    QString id() const override;
    QString name() const override;
    QString description() const override;
    // albert::PluginProvider
    QIcon icon() const override;
    const std::map<QString, albert::PluginSpec> &plugins() const override;
    bool setEnabled(const QString&, bool) override;  /// En-/Disable a plugin
    bool isEnabled(const QString&) override;

private:
    albert::PluginSpec parsePluginMetadata(const QString& path);
    albert::Plugin *load(albert::PluginSpec&);
    bool unload(albert::PluginSpec&);

    albert::ExtensionRegistry &registry_;
    std::map<QString,albert::PluginSpec> plugins_;
    std::vector<albert::PluginSpec*> frontend_plugins_;
    albert::Frontend *frontend_;
};

