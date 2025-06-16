// Copyright (c) 2024-2025 Manuel Schneider

#include "extensionplugin.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
using namespace albert::util;
using namespace albert;
using namespace std;

QString ExtensionPlugin::id() const
{ return loader().metadata().id; }

QString ExtensionPlugin::name() const
{ return loader().metadata().name; }

QString ExtensionPlugin::description() const
{ return loader().metadata().description; }

vector<Extension*> ExtensionPlugin::extensions()
{ return { this }; }
