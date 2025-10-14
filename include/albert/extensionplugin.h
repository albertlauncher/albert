// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/extension.h>
#include <albert/plugininstance.h>

namespace albert::util
{

///
/// Convenience base class for extension plugins.
///
/// Implements pure virtual functions of \ref Extension and \ref PluginInstance.
///
class ALBERT_EXPORT ExtensionPlugin : public PluginInstance,
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
