// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QStringList>

namespace albert
{

///
/// Common plugin metadata of all plugins.
///
class ALBERT_EXPORT PluginMetaData
{
public:

    /// Interface identifier.
    QString iid;

    /// GUID, no duplicates allowed.
    /// \remark To avoid name conflicts PluginLoaders should prefix
    ///         their plugins ids with the id of the loader
    QString id;

    /// https://semver.org/
    QString version;

    /// Human readable name.
    QString name;

    /// Brief, imperative description.
    QString description;

    /// Short form e.g. BSD-2.
    QString license;

    /// Browsable source, README, issues.
    QString url;

    /// The copyright holders.
    QStringList authors;

    /// Required libraries.
    QStringList runtime_dependencies;

    /// Required executables.
    QStringList binary_dependencies;

    /// Required plugins.
    /// \since 0.23
    QStringList plugin_dependencies;

    /// Third party credits and license notes.
    QStringList third_party_credits;

    /// List of supported platforms. Empty means all.
    QStringList platforms;

    /// The load type of the plugin.
    /// Some plugins have to be treated differently when loading.
    /// E.g. a frontends are an integral part of the app, there has to be
    /// exactly one frontend which will be loaded before any other plugins.
    /// Some other pluins cannot be safely unloaded at runtime (e.g. Python).
    enum class LoadType {
        User,          ///< Plugin should be user (un-)loadable.
        Frontend,      ///< Loading handled by the core. Requires Frontend interface.
    };

    /// \copybrief LoadType
    /// \sa Loadtype
    LoadType load_type{LoadType::User};

};

}
