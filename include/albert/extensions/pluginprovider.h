// Copyright (c) 2022 Manuel Schneider

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

enum class ALBERT_EXPORT PluginState {  /// The state of the plugin
    Invalid,                            /// Plugin does not fulfill the reqiurements
    Unloaded,                           /// The plugin is valid and ready to be loaded
    Loaded                              /// The plugin is loaded
};

struct ALBERT_EXPORT PluginMetaData {   /// Common plugin metadata
    QString iid;                        /// Interface identifier
    QString id;                         /// GUID, no duplicates allowed
    QString version;                    /// https://semver.org/
    QString name;                       /// Human readable name
    QString description;                /// Brief, imperative description
    QString long_description;           /// Elaborate, markdown formatted description (README.md)
    QString license;                    /// Short form e.g. BSD-2
    QString url;                        /// Browsable source, README, issues
    QStringList maintainers;            /// The current maintainers of the plugin []
    QStringList runtime_dependencies;   /// Required libraries []
    QStringList binary_dependencies;    /// Required executables []
    QStringList third_party_credits;    /// Third party credits and license notes []
    bool user = true;                   /// Users can (un-)load
};

class ALBERT_EXPORT PluginLoader
{
protected:
    PluginState state_;
    QString state_info_;
public:
    PluginLoader(const QString &path);
    const QString path;

    PluginState state() const;           /// @See albert::PluginState
    const QString &stateInfo() const;          /// Further information about the state of the plugin

    virtual PluginProvider *provider() const = 0;
    virtual const PluginMetaData &metaData() const = 0;
    virtual QWidget *makeInfoWidget() const = 0;
    virtual QString iconUrl() const = 0;
    virtual void load() = 0;
    virtual void unload() = 0;
};

class ALBERT_EXPORT PluginInfoWidget : public QWidget
{
public:
    explicit PluginInfoWidget(const PluginLoader&);
    QFormLayout *layout;

};

class ALBERT_EXPORT PluginProvider : virtual public Extension  /// Interface for plugin providing extensions
{
public:
    virtual std::vector<PluginLoader*> plugins() = 0;  /// The plugins provided
};

}
