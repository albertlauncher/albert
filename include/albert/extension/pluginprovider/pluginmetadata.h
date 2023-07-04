// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/export.h"
#include <QStringList>

namespace albert
{

/// Common plugin metadata of all plugins
class ALBERT_EXPORT PluginMetaData
{
public:
    QString iid;  ///< Interface identifier
    QString id;  ///< GUID, no duplicates allowed
    QString version;  ///< https://semver.org/
    QString name;  ///< Human readable name
    QString description;  ///< Brief, imperative description
    QString long_description;  ///< Elaborate, markdown formatted description (README.md)
    QString license;  ///< Short form e.g. BSD-2
    QString url;  ///< Browsable source, README, issues
    QStringList maintainers;  ///< The current maintainers of the plugin []
    QStringList runtime_dependencies;  ///< Required libraries []
    QStringList binary_dependencies;  ///< Required executables []
    QStringList third_party_credits;  ///< Third party credits and license notes []
    bool user = true;  ///< Users can (un-)load
    bool frontend = false;  ///< Plugin instance is of type Frontend
};

}
