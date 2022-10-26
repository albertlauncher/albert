// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "export.h"
#include "pluginprovider.h"

namespace albert
{

/// Qt requires plugin classes to be default constructible, inherit QObject and contain a Q_OBJECT macro. The metadata
/// is not accessible inside the QtPlugin by default. This class injects a reference to the plugin spec for DRY
/// principle while keeping default constructability.
/// @note Implementations have to ensure the QObject inheritance and contain the Q_OBJECT macro. This is a Qt MOC requirement.
class ALBERT_EXPORT Plugin : virtual public Extension
{
public:
    Plugin();
    QString id() const override;  /// The guid of the extension

    const PluginSpec &spec;  /// Plugin specification
};
}
