// Copyright (c) 2023-2025 Manuel Schneider

#include "albert.h"
#include "logging.h"
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
using std::filesystem::path;


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
{ return ::cacheLocation() / d->loader->metadata().id.toStdString(); }

filesystem::path PluginInstance::configLocation() const
{ return ::configLocation() / d->loader->metadata().id.toStdString(); }

filesystem::path PluginInstance::dataLocation() const
{ return ::dataLocation() / d->loader->metadata().id.toStdString(); }

unique_ptr<QSettings> PluginInstance::settings() const
{
    auto s = ::settings();
    s->beginGroup(d->loader->metadata().id);
    return s;
}

unique_ptr<QSettings> PluginInstance::state() const
{
    auto s = ::state();
    s->beginGroup(d->loader->metadata().id);
    return s;
}

const PluginLoader &PluginInstance::loader() const
{ return *d->loader; }

QString PluginInstance::readKeychain(const QString &key) const
{
    // Deletes itself
    auto job = new QKeychain::ReadPasswordJob(qApp->applicationName(), qApp);
    job->setKey(QString("%1.%2").arg(d->loader->metadata().id, key));

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

    job->setKey(QString("%1.%2").arg(d->loader->metadata().id, key));
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
                                                      loader().metadata().id,
                                                      QStandardPaths::LocateDirectory))
        data_locations.emplace_back(path.toStdString());
    return data_locations;
}
