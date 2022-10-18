// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "extension.h"
#include <QIcon>
#include <QString>
#include <map>

namespace albert
{

enum class PluginState {  /// The state of the plugin
    Error,
    Unloaded,
    Loading,
    Loaded,
};

enum class PluginType {  /// The state of the plugin
    None,
    User,
    Frontend
};
class PluginProvider;

struct PluginSpec {                    /// The specification of the plugin
    QString path;                      /// File path
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
    PluginType type;                   /// @see PluginType
    PluginState state;                 /// @see PluginState
    QString reason;                    /// Further information about the state the plugin is in
    PluginProvider *provider;          /// Provider of this plugin
};

/// Interface for plugin providing extensions
/// @note This is a QObject: Must be the first class inherited, no multiple inheritance
class ALBERT_EXPORT PluginProvider : public QObject, virtual public Extension
{
    Q_OBJECT

public:
    virtual QIcon icon() { return QIcon(":unknown"); };  /// Identifying icon
    virtual std::map<QString, PluginSpec> &plugins() = 0;  /// The plugins provided
    virtual bool isEnabled(const QString &id) = 0;  /// Autoload on start
    virtual void setEnabled(const QString &id, bool enabled = false) = 0;  /// En-/Disable autoload on start

signals:
    void pluginStateChanged(PluginSpec&);
};
}
