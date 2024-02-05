// Copyright (c) 2024 Manuel Schneider

#include "albert/extensionregistry.h"
#include "albert/plugin.h"
using namespace albert::plugin;
using namespace albert;
using namespace std;

QString ExtensionPlugin::id() const { return PluginInstance::id(); }

QString ExtensionPlugin::name() const { return PluginInstance::name(); }

QString ExtensionPlugin::description() const { return PluginInstance::description(); }

void ExtensionPlugin::initialize(ExtensionRegistry &registry, map<QString,PluginInstance*>)
{ registry.registerExtension(this); }

void ExtensionPlugin::finalize(ExtensionRegistry &registry)
{ registry.deregisterExtension(this); }
