// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension.h"
#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "plugininstanceprivate.h"
#include <QDir>
#include <QSettings>
using namespace albert;


PluginInstance::PluginInstance() : d(std::make_unique<PluginInstancePrivate>()) {}

PluginInstance::~PluginInstance() = default;

QString PluginInstance::id() const
{ return d->loader->metaData().id; }

QString PluginInstance::name() const
{ return d->loader->metaData().name; }

QString PluginInstance::description() const
{ return d->loader->metaData().description; }

static QDir make_dir(const QString &location, const QString &id)
{
    auto dir = QDir(location);
    if (!dir.cd(id)){
        if (!dir.mkpath(id))
            qFatal("Failed to create writable dir at: %s", qPrintable(dir.filePath(id)));
        if (!dir.cd(id))
            qFatal("Failed to cd to just created dir at: %s", qPrintable(dir.filePath(id)));
    }
    return dir;
}

QDir albert::PluginInstance::cacheDir() const
{ return make_dir(albert::cacheLocation(), id()); }

QDir albert::PluginInstance::configDir() const
{ return make_dir(albert::configLocation(), id()); }

QDir albert::PluginInstance::dataDir() const
{ return make_dir(albert::dataLocation(), id()); }

std::unique_ptr<QSettings> albert::PluginInstance::settings() const
{
    auto s = albert::settings();
    s->beginGroup(id());
    return s;
}

std::unique_ptr<QSettings> albert::PluginInstance::state() const
{
    auto s = albert::state();
    s->beginGroup(id());
    return s;
}

void PluginInstance::initialize(ExtensionRegistry *) {}

void PluginInstance::finalize(ExtensionRegistry *) {}

std::vector<Extension*> PluginInstance::extensions() { return {}; }

QWidget *PluginInstance::buildConfigWidget() { return nullptr; }

