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
#include <qt6keychain/keychain.h>
using namespace albert;
using namespace std;
using filesystem::path;


class PluginInstance::Private
{
public:
    PluginLoader *loader;

};


PluginInstance::PluginInstance():
    d(new Private{
        .loader = PluginLoader::current_loader,
    })
{}

PluginInstance::~PluginInstance() = default;

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

const PluginLoader &PluginInstance::loader() const
{ return *d->loader; }

void PluginInstance::readKeychain(const QString &key,
                                  function<void(const QString&)> onSuccess,
                                  function<void(const QString&)> onError) const
{
    auto job = new QKeychain::ReadPasswordJob(qApp->applicationName(), qApp);  // Deletes itself
    job->moveToThread(qApp->thread());

    job->setKey(QString("%1.%2").arg(d->loader->metadata().id, key));

    connect(job, &QKeychain::ReadPasswordJob::finished, this, [=] {
        if (job->error())
            onError(job->errorString());
        else
            onSuccess(job->textData());
    });

    job->start();
}

void PluginInstance::writeKeychain(const QString &key,
                                   const QString &value,
                                   function<void()> onSuccess,
                                   function<void(const QString&)> onError) const
{
    auto job = new QKeychain::WritePasswordJob(qApp->applicationName(), qApp);  // Deletes itself
    job->moveToThread(qApp->thread());

    job->setKey(QString("%1.%2").arg(d->loader->metadata().id, key));
    job->setTextData(value);

    connect(job, &QKeychain::Job::finished, this, [=] {
        if (job->error())
            onError(job->errorString());
        else
            onSuccess();
    });

    job->start();
}

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
