// Copyright (c) 2023 Manuel Schneider

#include "albert/extension/pluginprovider/pluginloader.h"
#include "pluginloaderprivate.h"
using namespace albert;


PluginLoader::PluginLoader(const QString &p):
    path(p), d(new PluginLoaderPrivate(this))
{
}

PluginLoader::~PluginLoader() = default;

PluginState PluginLoader::state() const { return d->state; }

const QString &PluginLoader::stateInfo() const { return d->state_info; }
