// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/frontend/frontend.h"
#include "albert/extensionregistry.h"
#include "appqueryhandler.h"
#include "hotkey.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
#include "qtpluginprovider.h"
#include "queryengine.h"
#include "rpcserver.h"
#include "scopedcrashindicator.h"
#include "settings/settingswindow.h"
#include "telemetry.h"
#include "terminalprovider.h"
#include "trayicon.h"
#include <QNetworkAccessManager>
#include <QPointer>

extern int main(int, char**);

class App
{
public:
    explicit App(const QStringList &additional_plugin_paths);
    ~App();

    void initialize();

    ScopedCrashIndicator crash_indicator;
    QNetworkAccessManager network_manager;
    RPCServer rpc_server; // Check for other instances first
    albert::ExtensionRegistry extension_registry;
    PluginRegistry plugin_registry;
    QueryEngine query_engine;
    QtPluginProvider plugin_provider;
    TerminalProvider terminal_provider;
    TrayIcon tray_icon;
    QPointer<SettingsWindow> settings_window;
    Hotkey hotkey;
    Telemetry telemetry;
    albert::Frontend *frontend;

    AppQueryHandler app_query_handler;
    PluginQueryHandler plugin_query_handler;

    static App *instance();

    void setFrontend(const QString &id);

private:

    void loadAnyFrontend();
    void applyPlatformWindowQuirks(albert::Frontend *);
    QString loadFrontend(QtPluginLoader *loader);

    friend int ::main(int, char**);
};
