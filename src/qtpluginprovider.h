// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/plugin/pluginprovider.h"
#include <QStringList>
#include <memory>
#include <vector>
namespace albert { class PluginLoader; }
class QtPluginLoader;

class QtPluginProvider : public albert::PluginProvider
{
public:

    explicit QtPluginProvider(QStringList additional_paths);
    ~QtPluginProvider();

    // albert::PluginProvider interface
    QString id() const override;
    QString name() const override;
    QString description() const override;
    std::vector<albert::PluginLoader*> plugins() override;
    std::vector<albert::PluginLoader*> frontendPlugins();

private:

    // on heap because vector requires to be move insertable
    std::vector<std::unique_ptr<QtPluginLoader>> plugin_loaders_;

};
