// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QStringList>

namespace albert
{

/// The load type of the plugin.
/// Some plugins have to be treated differently when loading.
/// E.g. a frontends are an integral part of the app, there has to be exactly one frontend which will be loaded before any other plugins.
/// Some other pluins cannot be safely unloaded at runtime (e.g. Python).
enum class LoadType {
    Frontend,   ///< Specifies wheter this plugin implements the Frontend interface.
    NoUnload,   ///< Specifies wheter this plugin should prohibit unloading.
    User,       ///< Specifies wheter this plugin should be user (un-)loadable.
};

/// Common plugin metadata of all plugins
class ALBERT_EXPORT PluginMetaData
{
public:
    QString iid;  ///< Interface identifier
    QString id;  ///< GUID, no duplicates allowed
    QString version;  ///< https://semver.org/
    QString name;  ///< Human readable name
    QString description;  ///< Brief, imperative description
    QString license;  ///< Short form e.g. BSD-2
    QString url;  ///< Browsable source, README, issues
    QStringList authors;  ///< The copyright holders []
    QStringList runtime_dependencies;  ///< Required libraries []
    QStringList binary_dependencies;  ///< Required executables []
    QStringList third_party_credits;  ///< Third party credits and license notes []
    QStringList platforms;  ///< List of supported platforms. {Linux, Darwin,Windows}*. Empty means all.
    LoadType load_type = LoadType::User;  ///< See LoadType
};

}
