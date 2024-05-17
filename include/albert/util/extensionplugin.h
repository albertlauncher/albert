// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension.h"
#include "albert/plugin/plugininstance.h"
#include <QObject>

namespace albert
{

///
/// Convenience base class for extension plugins.
///
/// Implements pure virtual functions of Extension and PluginInstance.
///
class ALBERT_EXPORT ExtensionPlugin : public QObject,
                                      public PluginInstance,
                                      virtual public Extension
{
public:
    /// Overrides Extension::id()
    /// @returns Plugin id
    QString id() const override;

    /// Overrides Extension::name()
    /// @returns Plugin name
    QString name() const override;

    /// Overrides Extension::description()
    /// @returns Plugin description
    QString description() const override;
};

}
