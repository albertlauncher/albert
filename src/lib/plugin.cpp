// Copyright (c) 2022 Manuel Schneider

#include "albert/extensions/pluginprovider.h"
#include "albert/plugin.h"
using namespace albert;

extern PluginSpec *current_spec_in_construction;

Plugin::Plugin() : spec(*current_spec_in_construction){}

QString Plugin::id() const
{
    return spec.id;
}

QString Plugin::name() const
{
    return spec.name;
}

QString Plugin::description() const
{
    return spec.description;
}