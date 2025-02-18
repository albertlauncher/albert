// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/extension.h>
#include <albert/plugininstance.h>

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

    /// Overrides PluginInstance::extensions()
    /// @returns `this`
    std::vector<albert::Extension*> extensions() override;

};

}
