// Copyright (c) 2024 Manuel Schneider

#include "extensionplugin.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
using namespace albert;
using namespace std;

QString ExtensionPlugin::id() const
{ return loader().metaData().id; }

QString ExtensionPlugin::name() const
{ return loader().metaData().name; }

QString ExtensionPlugin::description() const
{ return loader().metaData().description; }
