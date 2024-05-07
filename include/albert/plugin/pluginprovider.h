// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include <vector>

namespace albert
{
class PluginLoader;

///
/// Abstract plugin provider extension.
///
class ALBERT_EXPORT PluginProvider : virtual public Extension
{
public:

    /// The plugins provided by the provider.
    /// @return @copybrief plugins
    virtual std::vector<PluginLoader*> plugins() = 0;

protected:

    virtual ~PluginProvider();

};

}
