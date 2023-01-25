// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/extensions/pluginprovider.h"
#include <QLabel>
using namespace albert;


PluginLoader::PluginLoader(const QString &p) : path(p), state_(PluginState::Invalid) {}

PluginLoader::~PluginLoader() = default;

PluginState PluginLoader::state() const { return state_; }

const QString &PluginLoader::stateInfo() const { return state_info_; }


PluginInstance::~PluginInstance() = default;

void PluginInstance::initialize() {}

void PluginInstance::finalize() {}

QWidget *PluginInstance::buildConfigWidget() { return nullptr; }
