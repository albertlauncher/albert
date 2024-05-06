// Copyright (c) 2024 Manuel Schneider

#include "albert/util/extensionplugin.h"
using namespace albert;
using namespace std;

QString ExtensionPlugin::id() const
{ return PluginInstance::id(); }

QString ExtensionPlugin::name() const
{ return PluginInstance::name(); }

QString ExtensionPlugin::description() const
{ return PluginInstance::description(); }
