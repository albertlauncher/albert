// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#define EXPAND_STRINGIZE(s) STRINGIZE(s)
#define STRINGIZE(s) #s
#include "albert/config.h"
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
class PluginLoader;


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

    /// Global variable used for static dependency injection.
    /// Constructors are nice to have. However Qt plugins enforce default
    /// constructability. This conflicts the desire to have everything necessary
    /// in the constructor, especially the plugin id from the metadata.
    /// This hack emulates constructor injection and should be safe since plugin
    /// instantiation is serialized.
    static PluginLoader *instanciated_loader;

protected:

    virtual ~PluginInstance();

private:

    class Private;
    const std::unique_ptr<Private> d;
};

}

///
/// @brief Declare a class as native Albert plugin.
///
/// Sets the interface identifier to #ALBERT_PLUGIN_IID and uses the metadata
/// file named 'metadata.json' located at CMAKE_CURRENT_SOURCE_DIR.
///
/// @note This macro has to be put into the plugin class body.
/// The class this macro appears on must be default-constructible, inherit
/// QObject and contain the Q_OBJECT macro. There should be exactly one
/// occurrence of this macro in the source code for a plugin.
///
#define ALBERT_PLUGIN Q_PLUGIN_METADATA(IID ALBERT_PLUGIN_IID FILE "metadata.json")
