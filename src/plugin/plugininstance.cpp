// Copyright (c) 2023-2024 Manuel Schneider

#include "albert.h"
#include "logging.h"
#include "plugininstance.h"
#include "pluginloader.h"
#include "pluginmetadata.h"
#include "pluginregistry.h"
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <qt6keychain/keychain.h>
using namespace albert;
using namespace std;


class PluginInstance::Private
{
public:
    PluginLoader *loader;
    ExtensionRegistry *registry;

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
{ return ::cacheLocation() / d->loader->metaData().id.toStdString(); }

filesystem::path PluginInstance::configLocation() const
{ return ::configLocation() / d->loader->metaData().id.toStdString(); }

filesystem::path PluginInstance::dataLocation() const
{ return ::dataLocation() / d->loader->metaData().id.toStdString(); }

unique_ptr<QSettings> PluginInstance::settings() const
{
    auto s = ::settings();
    s->beginGroup(d->loader->metaData().id);
    return s;
}

unique_ptr<QSettings> PluginInstance::state() const
{
    auto s = ::state();
    s->beginGroup(d->loader->metaData().id);
    return s;
}

const PluginLoader &PluginInstance::loader() const
{ return *d->loader; }

QString PluginInstance::readKeychain(const QString &key) const
{
    // Deletes itself
    auto job = new QKeychain::ReadPasswordJob(qApp->applicationName(), qApp);
    job->setKey(QString("%1.%2").arg(d->loader->metaData().id, key));

    QEventLoop loop;
    QString value;

    QObject::connect(job, &QKeychain::ReadPasswordJob::finished, &loop,
                     [job, &loop, &value]
                     {
                         if (job->error())
                             DEBG << "Failed to read" << job->key() << ":" << job->errorString();
                         else
                             value = job->textData();
                         loop.quit();
                     });

    job->start();
    loop.exec();
    return value;
}

void PluginInstance::writeKeychain(const QString &key, const QString &value) const
{
    // Deletes itself
    auto job = new QKeychain::WritePasswordJob(qApp->applicationName(), qApp);

    job->setKey(QString("%1.%2").arg(d->loader->metaData().id, key));
    job->setTextData(value);

    QEventLoop loop;

    QObject::connect(job, &QKeychain::Job::finished,
                     &loop, [job, &loop]
                     {
                         if (job->error())
                             WARN << "Failed to write" << job->key() << ":" << job->errorString();
                         loop.quit();
                     });

    job->start();
    loop.exec();
}

vector<filesystem::path> PluginInstance::dataLocations() const
{
    vector<filesystem::path> data_locations;
    for (const auto &path : QStandardPaths::locateAll(QStandardPaths::AppDataLocation,
                                                      loader().metaData().id,
                                                      QStandardPaths::LocateDirectory))
        data_locations.emplace_back(path.toStdString());
    return data_locations;
}
