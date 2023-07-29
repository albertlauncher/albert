// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/pluginloader.h"
using namespace albert;

class PluginInstancePrivate
{
public:
    PluginLoader const * const loader{in_construction};
    inline static const PluginLoader *in_construction;
};

