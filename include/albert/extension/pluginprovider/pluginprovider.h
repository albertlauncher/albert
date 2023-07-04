// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include <vector>

namespace albert
{
class PluginLoader;

/// Interface class for a plugin providing extension
class ALBERT_EXPORT PluginProvider : virtual public Extension
{
public:
    virtual std::vector<PluginLoader*> plugins() = 0;  ///< The plugins provided
};

}
