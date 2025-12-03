// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QStringList>
#include <albert/export.h>

namespace albert
{

///
/// Common plugin metadata.
///
/// \ingroup core_plugin
///
class ALBERT_EXPORT PluginMetadata
{
public:

    ///
    /// Plugin interface identifier.
    ///
    /// The core app API version used.
    ///
    QString iid;

    ///
    /// Unique identifier.
    ///
    /// No duplicates allowed. To avoid name conflicts implementations should prefix their plugins
    /// ids with the id of the loader id.
    ///
    QString id;

    ///
    /// [Semantic version](https://semver.org/).
    ///
    QString version;

    ///
    /// Human readable name.
    ///
    QString name;

    ///
    /// Brief, imperative description.
    ///
    QString description;

    ///
    /// [SPDX short-form license identifier](https://spdx.org/licenses/).
    ///
    QString license;

    ///
    /// Browsable source.
    ///
    QString url;

    ///
    /// Online readme.
    ///
    QString readme_url;

    ///
    /// Available translations.
    ///
    QStringList translations;

    ///
    /// The copyright holders.
    ///
    QStringList authors;

    ///
    /// The current maintainers.
    ///
    QStringList maintainers;

    ///
    /// Required libraries.
    ///
    QStringList runtime_dependencies;

    ///
    /// Required executables.
    ///
    QStringList binary_dependencies;

    ///
    /// Required plugins.
    ///
    QStringList plugin_dependencies;

    ///
    /// Third party credits and license notes.
    ///
    QStringList third_party_credits;

    ///
    /// List of supported platforms.
    ///
    /// If empty all platforms are supported.
    ///
    QStringList platforms;

    ///
    /// The load type of the plugin.
    ///
    /// Some plugins have to be treated differently when loading.
    /// E.g. a frontends are an integral part of the app, there has to be
    /// exactly one frontend which will be loaded before any other plugins.
    /// Some other pluins cannot be safely unloaded at runtime (e.g. Python).
    ///
    enum class LoadType {
        User,          ///< Plugin should be user (un-)loadable.
        Frontend,      ///< Loading handled by the core. Requires Frontend interface.
    };


    LoadType load_type; ///< \copybrief LoadType

};

}
