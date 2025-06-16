// Copyright (c) 2024-2025 Manuel Schneider

#include "pluginloader.h"
using namespace albert;

thread_local PluginLoader *PluginLoader::current_loader;

PluginLoader::~PluginLoader() = default;
