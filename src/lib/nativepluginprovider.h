// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/pluginprovider.h"
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
    albert::ExtensionRegistry &registry_;
    std::vector<std::unique_ptr<NativePluginLoader>> plugins_;
    std::vector<NativePluginLoader*> frontend_plugins_;
    albert::Frontend *frontend_;
};

struct NativePluginMetaData : public albert::PluginMetaData
{
    bool frontend;
};

class NativePluginLoader : public albert::PluginLoader
{
public:
    NativePluginLoader(NativePluginProvider *provider, albert::ExtensionRegistry&, const QString &path);
    ~NativePluginLoader();

    albert::PluginInstance *instance();

    NativePluginProvider *provider() const override;
    NativePluginMetaData const &metaData() const override;
    QString iconUrl() const override;
    QWidget *makeInfoWidget() const override;
    void load() override;
    void unload() override;
private:
    NativePluginProvider *provider_;
    albert::ExtensionRegistry &registry_;
    albert::PluginInstance *instance_;
    NativePluginMetaData metadata_;
};

