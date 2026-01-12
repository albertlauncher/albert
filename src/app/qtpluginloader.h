// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "pluginloader.h"
#include "pluginmetadata.h"
#include <QPluginLoader>
#include <memory>
namespace albert { class PluginInstance; }
class QTranslator;

class QtPluginLoader final : public albert::PluginLoader
{
public:

    QtPluginLoader(const QString &path);
    ~QtPluginLoader();

    QString path() const override;
    const albert::PluginMetadata &metadata() const override;
    void load() override;
    void unload() override;
    albert::PluginInstance *instance() override;

private:

    QPluginLoader loader_;
    albert::PluginMetadata metadata_;
    albert::PluginInstance *instance_;
    std::unique_ptr<QTranslator> translator_;

};
