// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/extensions/pluginprovider.h"
#include "albert/plugininstance.h"
#include <vector>
namespace albert {
class Frontend;
class PluginInstance;
class ExtensionRegistry;
}
class NativePluginLoader;


class NativePluginProvider : public albert::PluginProvider
{
public:
    explicit NativePluginProvider(albert::ExtensionRegistry&, const QStringList& additional_paths);
    ~NativePluginProvider() override;

    void loadFrontend();

    albert::Frontend *frontend();
    const std::vector<NativePluginLoader*> &frontendPlugins();
    void setFrontend(uint);

protected:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    std::vector<albert::PluginLoader*> plugins() override;

private:
    std::vector<std::unique_ptr<NativePluginLoader>> plugins_;
    std::vector<NativePluginLoader*> frontend_plugins_;
    albert::Frontend *frontend_;
};

class NativePluginMetaData : public albert::PluginMetaData
{
public:
    bool frontend;
};

class NativePluginLoader : public albert::PluginLoader
{
public:
    NativePluginLoader(NativePluginProvider *provider, albert::ExtensionRegistry&, const QString &path);
    ~NativePluginLoader();


    NativePluginProvider *provider() const override;
    NativePluginMetaData const &metaData() const override;
    albert::NativePluginInstance *instance() const override;
    void load() override;
    void unload() override;
private:
    NativePluginProvider *provider_;
    albert::ExtensionRegistry &registry_;
    albert::NativePluginInstance *instance_;
    NativePluginMetaData metadata_;
};

