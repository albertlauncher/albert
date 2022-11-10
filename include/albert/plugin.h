// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "config.h"
#include "export.h"
#include "pluginprovider.h"

namespace albert
{
/// The plugin entry point class.
/// Qt requires plugin classes to be default constructible, inherit QObject and
/// contain a Q_OBJECT and the Q_PLUGIN_METADATA(…) macro. This is a Qt MOC
/// requirement and can not be omitted. Also the metadata is not accessible
/// inside the QtPlugin by default. This class injects a reference to the
/// plugin spec while keeping default constructability.
/// @note Use the ALBERT_PLUGIN macro for convenience use
class ALBERT_EXPORT Plugin : virtual public Extension
{
public:
    Plugin();
    QString id() const override;  /// The guid of the extension

    const PluginSpec &spec;  /// Plugin specification
};
}

#define ALBERT_PLUGIN Q_PLUGIN_METADATA(IID ALBERT_IID FILE "metadata.json")