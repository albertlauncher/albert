// Copyright (c) 2023 Manuel Schneider

#pragma once

#if defined ALBERT_PLUGIN_ID

#include "albert/config.h"
#include "albert/extension/pluginprovider/plugininstance.h"
#include "extension.h"
#include <QObject>
#include <QStringLiteral>



/// Declare a class as Albert plugin.
///
/// This is a convenience macro that calls the required Qt macro to make a
/// class a QtPlugin with defaults appropiate for an Albert plugin. It uses the
/// global ALBERT_IID define to set the interface id and assumes there is a
/// file named 'metadata.json' located in the root of the plugin source dir
/// containing the metadata as of the Albert plugin. This file may be
/// handcrafted but it is recommended to use the `albert_plugin` CMake macro
/// which automatically generates this file.
///
/// @note This macro has to be put into the plugin class declaration body.
/// The class this macro appears on must be default-constructible, inherit
/// QObject and contain the Q_OBJECT macro. There should be exactly one
/// occurrence of this macro in the source code for a plugin.
#define ALBERT_PLUGIN Q_PLUGIN_METADATA(IID ALBERT_PLUGIN_IID FILE "metadata.json")

namespace albert::plugin
{

template <class QOBJECT = QObject>
class ALBERT_EXPORT Plugin : public QOBJECT, public albert::PluginInstance
{
public:
    /// Override using the plugin specification
    QString id() const override { return QStringLiteral(ALBERT_PLUGIN_ID); }
};


/// Declare a class inheriting albert::Extension as Albert plugin.
///
/// This is a convenience macro extending ALBERT_PLUGIN. Additionally it
/// overrides the virtual functions of albert::Extension.
///
/// @see ALBERT_PLUGIN
template <class QOBJECT = QObject>
class ALBERT_EXPORT ExtensionPlugin : public albert::plugin::Plugin<QOBJECT>, virtual public albert::Extension
{
public:
    /// Override using the plugin specification
    QString id() const override { return Plugin<QOBJECT>::id(); }

    /// Override using the plugin specification
    QString name() const override { return QStringLiteral(ALBERT_PLUGIN_NAME); }

    /// Override using the plugin specification
    QString description() const override { return QStringLiteral(ALBERT_PLUGIN_DESCRIPTION); }

    /// Override returning self
    std::vector<Extension*> extensions() override { return {this}; }

    using Extension::settings;  // Disambiguate calls to settings
};

}

#endif
