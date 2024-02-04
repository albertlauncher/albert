// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QString>
#include <map>
#include <memory>
class QDir;
class QSettings;
class QWidget;

namespace albert
{
class ExtensionRegistry;

///
/// Abstract plugin instance class.
///
/// The base class every plugin has to inherit.
///
class ALBERT_EXPORT PluginInstance
{
public:
    PluginInstance();

    /// The plugin identifier.
    /// Taken from the metadata.
    QString id() const;

    /// The human readable plugin name.
    /// Taken from the metadata.
    QString name() const;

    /// Brief description of the plugin.
    /// Taken from the metadata.
    QString description() const;

    /// The recommended cache location.
    /// Created if necessary.
    QDir cacheDir() const;

    /// The recommended config location.
    /// Created if necessary.
    QDir configDir() const;

    /// The recommended data location.
    /// Created if necessary.
    QDir dataDir() const;

    /// Persistent plugin settings utilizing QSettings.
    /// Configured to use a section titled <plugin-id>.
    std::unique_ptr<QSettings> settings() const;

    /// Persistent plugin state utilizing QSettings.
    /// Configured to use a section titled <plugin-id>.
    /// \since 0.23
    std::unique_ptr<QSettings> state() const;

    /// The initialization function.
    /// \param registry The extension registry.
    /// \param instances The dependencies of the plugin.
    /// \since 0.23
    virtual void initialize(ExtensionRegistry &registry, std::map<QString,PluginInstance*> dependencies);

    /// The finalization function.
    /// \param registry The extension registry.
    /// \since 0.23
    virtual void finalize(ExtensionRegistry &registry);

    /// Config widget factory.
    virtual QWidget *buildConfigWidget();

protected:

    virtual ~PluginInstance();

private:

    class Private;
    const std::unique_ptr<Private> d;
};

}
