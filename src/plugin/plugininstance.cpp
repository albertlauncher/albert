// Copyright (c) 2023-2024 Manuel Schneider

#include "albert.h"
#include "plugininstance.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginregistry.h"
#include <QCoreApplication>
#include <QSettings>
using namespace albert;
using namespace std;


class PluginInstance::Private
{
public:
    PluginLoader *loader;
    albert::ExtensionRegistry *registry;

};

PluginInstance::PluginInstance():
    d(new Private{
        .loader = PluginRegistry::staticDI.loader,
        .registry = PluginRegistry::staticDI.registry
    })
{}

PluginInstance::~PluginInstance() = default;

vector<Extension*> PluginInstance::extensions() { return {}; }

QWidget *PluginInstance::buildConfigWidget() { return nullptr; }

filesystem::path PluginInstance::cacheLocation() const
{ return albert::cacheLocation() / d->loader->metaData().id.toStdString(); }

filesystem::path PluginInstance::configLocation() const
{ return albert::configLocation() / d->loader->metaData().id.toStdString(); }

filesystem::path PluginInstance::dataLocation() const
{ return albert::dataLocation() / d->loader->metaData().id.toStdString(); }

unique_ptr<QSettings> albert::PluginInstance::settings() const
{
    auto s = albert::settings();
    s->beginGroup(d->loader->metaData().id);
    return s;
}

unique_ptr<QSettings> albert::PluginInstance::state() const
{
    auto s = albert::state();
    s->beginGroup(d->loader->metaData().id);
    return s;
}

const PluginLoader &PluginInstance::loader() const
{ return *d->loader; }

ExtensionRegistry &PluginInstance::registry()
{ return *d->registry; }
