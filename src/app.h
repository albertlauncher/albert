// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensionregistry.h"
#include "albert/extensions/queryhandler.h"
#include "hotkey.h"
#include "nativepluginprovider.h"
#include "pluginregistry.h"
#include "queryengine.h"
#include "rpcserver.h"
#include "settings/settingswindow.h"
#include "telemetry.h"
#include "terminalprovider.h"
#include "trayicon.h"
#include <QPointer>

class App : public albert::IndexQueryHandler
{
public:
    explicit App(const QStringList &additional_plugin_paths);
    ~App() override;

    void initialize();

    // IndexQueryHandler
    QString id() const override;
    QString name() const override;
    QString description() const override;
    void updateIndexItems() override;

    RPCServer rpc_server; // Check for other instances first
    albert::ExtensionRegistry extension_registry;
    PluginRegistry plugin_registry;
    QueryEngine query_engine;
    NativePluginProvider plugin_provider;
    TerminalProvider terminal_provider;
    TrayIcon tray_icon;
    QPointer<SettingsWindow> settings_window;
    Hotkey hotkey;
    Telemetry telemetry;
};
