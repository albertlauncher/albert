// Copyright (C) 2014-2018 Manuel Schneider

#include <QStandardPaths>
#include <QCoreApplication>
#include "albert/plugin.h"

class Core::PluginPrivate
{
public:
    QString id;
    std::unique_ptr<QSettings> settings;
};

Core::Plugin::Plugin(const QString &id) : d(new PluginPrivate)
{
    d->id = id;
    d->settings.reset(new QSettings(qApp->applicationName()));
    d->settings->beginGroup(d->id);


//        // Create a settings instance
//        d->settings.reset(new QSettings(configLocation().filePath("config"),
//                                        QSettings::Format::IniFormat));

//        // Define a function to port the config from one settings to another
//        std::function<void(QSettings &from, QSettings &to)> portRecursively =
//                [&portRecursively](QSettings &from, QSettings &to){

//            for ( const QString & key : from.childKeys() ) {
//                to.setValue(key, from.value(key));
//                from.remove(key);
//            }

//            for ( const QString & group : from.childGroups() ){
//                from.beginGroup(group);
//                to.beginGroup(group);
//                portRecursively(from, to);
//                to.endGroup();
//                from.endGroup();
//            }
//        };

//        // Port the settings from the global settings to the new one
//        QSettings oldSettings(qApp->applicationName());
//        oldSettings.beginGroup(d->id);
//        portRecursively(oldSettings, *d->settings);


}

Core::Plugin::~Plugin()
{

}

const QString &Core::Plugin::id() const
{
    return d->id;
}

QDir Core::Plugin::cacheLocation() const
{
    QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    if ( !cacheDir.exists(d->id) )
        cacheDir.mkdir(d->id);
    cacheDir.cd(d->id);
    return cacheDir;
}

QDir Core::Plugin::configLocation() const
{
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if ( !configDir.exists(d->id) )
        configDir.mkdir(d->id);
    configDir.cd(d->id);
    return configDir;
}

QDir Core::Plugin::dataLocation() const
{
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if ( !dataDir.exists(d->id) )
        dataDir.mkdir(d->id);
    dataDir.cd(d->id);
    return dataDir;
}

QSettings &Core::Plugin::settings() const
{

    return *d->settings;
}
