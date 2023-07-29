// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>
#include <memory>
#include <vector>
class QSettings;
class QDir;
class QWidget;
class PluginInstancePrivate;

namespace albert
{
class Extension;
class ExtensionRegistry;

/// Abstract plugin instance class.
/// Instanciated by a PluginLoader.
class ALBERT_EXPORT PluginInstance
{
public:
    PluginInstance();
    virtual ~PluginInstance();

    /// The plugin identifier.
    /// Taken from the metadata.
    QString id() const;

    /// The human readable plugin name.
    /// Taken from the metadata.
    QString name() const;

    /// Brief description of the plugin.
    /// Taken from the metadata.
    QString description() const;

    virtual void initialize(ExtensionRegistry*);  ///< The initialization function.
    virtual void finalize(ExtensionRegistry*);  ///<  The initialization function.
    virtual std::vector<Extension*> extensions();  ///< The extensions this plugin provides.
    virtual QWidget *buildConfigWidget();  ///< Config widget factory.

    std::unique_ptr<QDir> cacheDir() const;  ///< The recommended cache location.
    std::unique_ptr<QDir> configDir() const;  ///< The recommended config location.
    std::unique_ptr<QDir> dataDir() const;  ///< The recommended data location.
    std::unique_ptr<QSettings> settings() const;  ///< Prepared settings object.

private:
    const std::unique_ptr<PluginInstancePrivate> d;
};

}
