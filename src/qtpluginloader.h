// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include <QPluginLoader>
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
    QtPluginLoader(const QtPluginProvider &provider, const QString &path);
    ~QtPluginLoader();

    const albert::PluginProvider &provider() const override;
    const albert::PluginMetaData &metaData() const override;

    QString load() override;
    QString unload() override;
    albert::PluginInstance *instance() const override;

    // Used to load frontends in advance (bypass registry).
    QString loadUnregistered(albert::ExtensionRegistry *registry, bool = true);

private:
    QPluginLoader loader;
    const QtPluginProvider &provider_;
    albert::PluginInstance *instance_;
    albert::PluginMetaData metadata_;
};

