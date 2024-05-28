// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/plugin/plugininstance.h"
#include "albert/plugin/pluginloader.h"
#include "albert/plugin/pluginmetadata.h"
#include "albert/util.h"
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

QString PluginInstance::cacheLocation() const
{ return QDir(albert::cacheLocation()).filePath(d->loader->metaData().id); }

QString PluginInstance::configLocation() const
{ return QDir(albert::configLocation()).filePath(d->loader->metaData().id); }

QString PluginInstance::dataLocation() const
{ return QDir(albert::dataLocation()).filePath(d->loader->metaData().id); }

QDir PluginInstance::createOrThrow(const QString &path)
{
    auto dir = QDir(path);
    if (!dir.exists() && !dir.mkpath("."))
        throw runtime_error("Could not create directory: " + path.toStdString());
    return dir;
}

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

QWidget *PluginInstance::buildConfigWidget()
{ return nullptr; }
