// Copyright (c) 2023-2025 Manuel Schneider

#include "albert/app.h"
#include "plugininstance.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginregistry.h"
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <filesystem>
using namespace albert;
using namespace std;
using filesystem::path;

class PluginInstance::Private
{
public:
    PluginLoader *loader;
};

PluginInstance::PluginInstance() :
    d(new Private{
        .loader = PluginLoader::current_loader,
    })
{}

PluginInstance::~PluginInstance() = default;

void PluginInstance::initialize() { emit initialized(); }

vector<Extension*> PluginInstance::extensions() { return {}; }

QWidget *PluginInstance::buildConfigWidget() { return nullptr; }

filesystem::path PluginInstance::cacheLocation() const
{ return App::cacheLocation() / d->loader->metadata().id.toStdString(); }

filesystem::path PluginInstance::configLocation() const
{ return App::configLocation() / d->loader->metadata().id.toStdString(); }

filesystem::path PluginInstance::dataLocation() const
{ return App::dataLocation() / d->loader->metadata().id.toStdString(); }

unique_ptr<QSettings> PluginInstance::settings() const
{
    auto s = App::settings();
    s->beginGroup(d->loader->metadata().id);
    return s;
}

unique_ptr<QSettings> PluginInstance::state() const
{
    auto s = App::state();
    s->beginGroup(d->loader->metadata().id);
    return s;
}

const PluginLoader &PluginInstance::loader() const { return *d->loader; }

vector<filesystem::path> PluginInstance::dataLocations() const
{
    vector<filesystem::path> data_locations;
    const auto paths = QStandardPaths::locateAll(QStandardPaths::AppDataLocation,
                                                 loader().metadata().id,
                                                 QStandardPaths::LocateDirectory);
    for (const auto &path : paths)
        data_locations.emplace_back(path.toStdString());
    return data_locations;
}
