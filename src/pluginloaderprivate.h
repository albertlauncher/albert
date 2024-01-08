// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/pluginloader.h"

class albert::PluginLoader::PluginLoaderPrivate
{
public:

    PluginLoader *q;
    QString state_info{};
    PluginState state{PluginState::Unloaded};

    PluginLoaderPrivate(PluginLoader *l);

    void setState(PluginState s, QString info = {});

    QString load(ExtensionRegistry *registry);

    QString unload(ExtensionRegistry *registry);
};
