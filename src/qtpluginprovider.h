// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/pluginprovider.h"
#include "qtpluginloader.h"
#include <QStringList>
namespace albert {
class Frontend;
class PluginLoader;
}

class QtPluginProvider : public albert::PluginProvider
{
public:
    explicit QtPluginProvider(QStringList additional_paths);

    QString id() const override;
    QString name() const override;
    QString description() const override;
    std::vector<albert::PluginLoader*> plugins() override;

    std::vector<QtPluginLoader*> frontendPlugins();

private:
    QStringList paths_;
    std::vector<std::unique_ptr<QtPluginLoader>> plugins_;
};
