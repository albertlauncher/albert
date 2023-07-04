// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/pluginprovider.h"
#include <vector>
namespace albert {
class Frontend;
class ExtensionRegistry;
class PluginLoader;
}
class QtPluginLoader;


class QtPluginProvider : public albert::PluginProvider
{
public:
    explicit QtPluginProvider(const QStringList& additional_paths);
    ~QtPluginProvider() override;

    void loadFrontend();

    albert::Frontend *frontend();
    const std::vector<QtPluginLoader*> &frontendPlugins();
    void setFrontend(uint);

protected:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    std::vector<albert::PluginLoader*> plugins() override;

private:
    std::vector<std::unique_ptr<QtPluginLoader>> plugins_;
    std::vector<QtPluginLoader*> frontend_plugins_;
    albert::Frontend *frontend_;
};
