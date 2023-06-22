// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extensionregistry.h"
#include "hotkey.h"
#include "appqueryhandler.h"
#include "pluginqueryhandler.h"
#include "nativepluginprovider.h"
#include "pluginregistry.h"
#include "queryengine.h"
#include "rpcserver.h"
#include "settings/settingswindow.h"
#include "telemetry.h"
#include "terminalprovider.h"
#include "trayicon.h"
#include <QNetworkAccessManager>
#include <QPointer>

class App
{
public:
    explicit App(const QStringList &additional_plugin_paths);
    ~App();

    void initialize();

    QNetworkAccessManager network_manager;
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

    AppQueryHandler app_query_handler;
    PluginQueryHandler plugin_query_handler;
};
