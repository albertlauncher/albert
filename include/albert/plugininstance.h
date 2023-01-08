// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "config.h"
#include "extension.h"
#include "export.h"
#define ALBERT_PLUGIN Q_PLUGIN_METADATA(IID ALBERT_IID FILE "metadata.json")
class NativePluginMetaData;

namespace albert
{
class ExtensionRegistry;
/// The plugin entry point class.
/// Qt requires plugin classes to be default constructible, inherit QObject and
/// contain a Q_OBJECT and the Q_PLUGIN_METADATA(…) macro. This is a Qt MOC
/// requirement and can not be omitted. Also the metadata is not accessible
/// inside the QtPlugin by default. This class inherits QObject and injects a
/// reference to the plugin spec while keeping default constructability.
/// @note Use the ALBERT_PLUGIN macro for convenience
class ALBERT_EXPORT PluginInstance : public QObject
{
public:
    ~PluginInstance() override;

protected:
    PluginInstance();
    const NativePluginMetaData &metaData() const;  /// Plugin metadata
    albert::ExtensionRegistry &registry();  /// Use this to register additional extensions

private:
    class Private;
    std::unique_ptr<Private> d;
};


/// Convenience class for extension plugins
/// Inherits extension and implements virtual methods using the metadata
/// Also plugins of type extension are automatically registered
class ALBERT_EXPORT ExtensionPlugin : public PluginInstance, virtual public Extension
{
public:
    QString id() const override;  /// Override using the plugin specification
    QString name() const override;  /// Override using the plugin specification
    QString description() const override;  /// Override using the plugin specification
};
}
