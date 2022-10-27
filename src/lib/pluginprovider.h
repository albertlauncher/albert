// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/pluginprovider.h"  //  disambiguation
#include <QIcon>
#include <QString>
#include <map>

class PluginProvider : public albert::PluginProvider
{
public:
    explicit PluginProvider();
    ~PluginProvider() override;
    void findPlugins(const QStringList &paths);

    // Interfaces
    QString id() const override;
    QIcon icon() override;
    std::map<QString, albert::PluginSpec> &plugins() override;
    bool isEnabled(const QString &id) override;
    void setEnabled(const QString &id, bool enabled) override;

    bool loadPlugin(const QString &id);
    void unloadPlugin(const QString &id);

private:
    std::map<QString, albert::PluginSpec> specs;

    void loadEnabledPlugins();
    albert::PluginSpec parsePluginMetadata(QString path);

//    // ALL plugins
//    std::vector<std::unique_ptr<NativePluginSpec>> plugins_;
//    std::map<QString,NativePluginSpec*> plugin_index;
//    // VALID plugins
//    std::map<NativePluginSpec*,std::set<NativePluginSpec*>> transitive_dependencies;
//    std::map<NativePluginSpec*,std::set<NativePluginSpec*>> transitive_dependees;
};

