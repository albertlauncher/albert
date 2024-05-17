// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/plugin/pluginloader.h"
#include "albert/plugin/pluginmetadata.h"
#include <QPluginLoader>
namespace albert { class PluginInstance; }
class QTranslator;

class QtPluginLoader : public albert::PluginLoader
{
public:

    QtPluginLoader(const QString &path);
    ~QtPluginLoader();

    QString path() const override;
    const albert::PluginMetaData &metaData() const override;
    void load() override;
    void unload() override;
    albert::PluginInstance *createInstance() override;

private:

    QPluginLoader loader_;
    albert::PluginMetaData metadata_;
    albert::PluginInstance *instance_;
    std::unique_ptr<QTranslator> translator;

};
