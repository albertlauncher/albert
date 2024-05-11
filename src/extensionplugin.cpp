// Copyright (c) 2024 Manuel Schneider

#include "albert/util/extensionplugin.h"
#include "albert/plugin/pluginloader.h"
#include "albert/plugin/pluginmetadata.h"
using namespace albert;
using namespace std;

QString ExtensionPlugin::id() const
{ return loader.metaData().id; }

QString ExtensionPlugin::name() const
{ return loader.metaData().name; }

QString ExtensionPlugin::description() const
{ return loader.metaData().description; }
