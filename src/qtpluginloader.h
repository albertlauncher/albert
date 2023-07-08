// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include <QFutureWatcher>
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
    albert::PluginInstance *instance() const override;

    QString load(albert::ExtensionRegistry*) override;
    QString unload(albert::ExtensionRegistry*) override;

    // Sync load (instantiate (implicit load), initialize, register)
    void load_(albert::ExtensionRegistry*);
    void unload_(albert::ExtensionRegistry*);

private:
    QPluginLoader loader;
    const QtPluginProvider &provider_;
    albert::PluginInstance *instance_;
    albert::PluginMetaData metadata_;
    QFutureWatcher<bool> watcher_;
};

