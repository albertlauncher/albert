// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/extension.h>
#include <albert/plugininstance.h>

namespace albert
{

///
/// Extension plugin base class.
///
/// Implements pure virtual functions of \ref Extension and \ref PluginInstance.
///
/// \ingroup util_plugin
///
class ALBERT_EXPORT ExtensionPlugin : public PluginInstance,
                                      virtual public Extension
{
public:
    ///
    /// Returns \ref PluginMetadata::id.
    ///
    QString id() const override;

    ///
    /// Returns \ref PluginMetadata::name.
    ///
    QString name() const override;

    ///
    /// Returns \ref PluginMetadata::description.
    ///
    QString description() const override;

    ///
    /// Returns `this` extension.
    ///
    std::vector<albert::Extension*> extensions() override;

};

}
