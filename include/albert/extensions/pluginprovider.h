// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "../extension.h"
#include <QStringList>
#include <map>
class QIcon;

namespace albert
{
class PluginProvider;


struct PluginSpec {                    /// The specification of the plugin
    QString path;                      /// File path
    QString iid;                       /// Interface identifier
    QString id;                        /// GUID, no duplicates allowed
    QString version;                   /// https://semver.org/
    QString name;                      /// Human readable name
    QString description;               /// Human readable, brief, imperative description
    QString license;                   /// Short form e.g. BSD-2
    QString url;                       /// Browsable source, README, issues
    QStringList maintainers;           /// The current maintainers of the plugin []
    QStringList authors;               /// Contributors, authors, etc [autogenerate with git shortlog]
    QStringList plugin_dependencies;   /// Inter plugin dependencies []
    QStringList runtime_dependencies;  /// Required libraries []
    QStringList binary_dependencies;   /// Required executables []
    QStringList third_party;           /// Third party credits and license notes []
    enum class Type {                  /// The state of the plugin
        None,                          /// Plugin without an interface defined by the core app
        User,                          /// Is of type albert::Plugin. User can set load state.
        Frontend                       /// Is of type albert::Frontend.
    } type;                            /// @see PluginType
    enum class State {                 /// The state of the plugin
        Error,                         /// Errors orccured while loading or reading the metadata
        Unloaded,                      /// The plugin is valid and ready to be loaded
        Loaded,                        /// The plugin is loaded and ready for use
    } state;                           /// @see PluginState
    QString reason;                    /// Further information about the state the plugin is in
    PluginProvider *provider;          /// Provider of this plugin
};


/// Interface for plugin providing extensions
/// @note This is a QObject: Must be the first class inherited, no multiple inheritance
class ALBERT_EXPORT PluginProvider : virtual public Extension
{
public:
    virtual QIcon icon() const = 0;  /// Identifying icon
    virtual const std::map<QString,PluginSpec> &plugins() const = 0;  /// The plugins provided
    virtual bool setEnabled(const QString&, bool) = 0;  /// En-/Disable a plugin
    virtual bool isEnabled(const QString&) = 0;  /// Return plugins user set enabled state
};

}
