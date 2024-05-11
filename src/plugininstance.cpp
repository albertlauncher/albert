// Copyright (c) 2023-2024 Manuel Schneider

#include <QCoreApplication>
#include <QSettings>
#include <QTranslator>
#include "albert/albert.h"
#include "albert/plugin/plugininstance.h"
#include "albert/plugin/pluginloader.h"
#include "albert/plugin/pluginmetadata.h"
#include "pluginregistry.h"
using namespace albert;
using namespace std;

PluginInstance::PluginInstance():
    loader(*PluginRegistry::staticDI.loader),
    registry(*PluginRegistry::staticDI.registry)
{}

PluginInstance::~PluginInstance() = default;

QString PluginInstance::cacheLocation() const
{ return QDir(albert::cacheLocation()).filePath(loader.metaData().id); }

QString PluginInstance::configLocation() const
{ return QDir(albert::configLocation()).filePath(loader.metaData().id); }

QString PluginInstance::dataLocation() const
{ return QDir(albert::dataLocation()).filePath(loader.metaData().id); }

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
    s->beginGroup(loader.metaData().id);
    return s;
}

unique_ptr<QSettings> albert::PluginInstance::state() const
{
    auto s = albert::state();
    s->beginGroup(loader.metaData().id);
    return s;
}

std::unique_ptr<QTranslator, function<void(QTranslator*)>>
PluginInstance::translator(bool install) const
{
    auto deleter = [install](QTranslator *t) {
        if (install)
            QCoreApplication::removeTranslator(t);
        delete t;
    };

    auto t = unique_ptr<QTranslator, decltype(deleter)>(new QTranslator, deleter);

    if (!t->load(QLocale(),
                 QString("%1.%2").arg(qApp->applicationName(), loader.metaData().id),
                 "_", ":/i18n"))
        return {};

    if (install)
        QCoreApplication::installTranslator(t.get());

    return t;
}

QWidget *PluginInstance::buildConfigWidget()
{ return nullptr; }
