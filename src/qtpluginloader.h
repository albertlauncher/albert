// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extension/pluginprovider/plugininstance.h"
#include "qtpluginprovider.h"
#include <vector>
namespace albert {
class Frontend;
class ExtensionRegistry;
class PluginProvider;
}
class QtPluginProvider;


class QtPluginLoader : public albert::PluginLoader
{
public:
    QtPluginLoader(QtPluginProvider *provider, const QString &path);
    ~QtPluginLoader();

    QtPluginProvider *provider() const override;
    albert::PluginMetaData const &metaData() const override;
    albert::PluginInstance *instance() const override;
    void load() override;
    void unload() override;

private:
    QtPluginProvider *provider_;
    albert::PluginInstance *instance_;
    albert::PluginMetaData metadata_;
};

