// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "../extension.h"
#include <QWidget>
#include <QFormLayout>
#include <QStringList>
#include <map>
#include <set>
class PluginRegistry;

namespace albert
{
class PluginProvider;

enum class ALBERT_EXPORT PluginState {  ///< The state of the plugin
    Invalid,                            ///< Plugin does not fulfill the reqiurements
    Unloaded,                           ///< The plugin is valid and ready to be loaded
    Loaded                              ///< The plugin is loaded
};

struct ALBERT_EXPORT PluginMetaData {   ///< Common plugin metadata
    QString iid;                        ///< Interface identifier
    QString id;                         ///< GUID, no duplicates allowed
    QString version;                    ///< https://semver.org/
    QString name;                       ///< Human readable name
    QString description;                ///< Brief, imperative description
    QString long_description;           ///< Elaborate, markdown formatted description (README.md)
    QString license;                    ///< Short form e.g. BSD-2
    QString url;                        ///< Browsable source, README, issues
    QStringList maintainers;            ///< The current maintainers of the plugin []
    QStringList runtime_dependencies;   ///< Required libraries []
    QStringList binary_dependencies;    ///< Required executables []
    QStringList third_party_credits;    ///< Third party credits and license notes []
    bool user = true;                   ///< Users can (un-)load
};

class ALBERT_EXPORT PluginInstance
{
public:
    virtual ~PluginInstance();
    virtual void initialize();
    virtual void finalize();
    virtual QWidget *buildConfigWidget();
};

class ALBERT_EXPORT PluginLoader
{
public:
    PluginLoader(const QString &path);
    virtual ~PluginLoader();

    const QString path;
    PluginState state() const;         ///< @See albert::PluginState
    const QString &stateInfo() const;  ///< Further information about the state of the plugin

    virtual PluginProvider *provider() const = 0;
    virtual const PluginMetaData &metaData() const = 0;
    virtual PluginInstance *instance() const = 0;
    virtual void load() = 0;
    virtual void unload() = 0;
protected:
    PluginState state_;
    QString state_info_;
};

class ALBERT_EXPORT PluginProvider : virtual public Extension  /// Interface for plugin providing extensions
{
public:
    virtual std::vector<PluginLoader*> plugins() = 0;  /// The plugins provided
};

}
