// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extensionregistry.h"
#include "appqueryhandler.h"
#include "hotkey.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
#include "qtpluginprovider.h"
#include "queryengine.h"
#include "rpcserver.h"
#include "session.h"
#include "settings/settingswindow.h"
#include "telemetry.h"
#include "terminalprovider.h"
#include "trayicon.h"
#include <QNetworkAccessManager>
#include <QPointer>
namespace albert {
class PluginLoader;
class Frontend;
}

extern int main(int, char**);

class App : QObject
{
    Q_OBJECT
public:
    explicit App(const QStringList &additional_plugin_paths, bool load_enabled);

    void initialize();
    void finalize();

    QNetworkAccessManager network_manager;
    RPCServer rpc_server; // Check for other instances first
    albert::ExtensionRegistry extension_registry;
    PluginRegistry plugin_registry;
    QueryEngine query_engine;
    QtPluginProvider plugin_provider;
    TerminalProvider terminal_provider;
    TrayIcon tray_icon;
    QPointer<SettingsWindow> settings_window;
    Hotkey hotkey;  // must be unwinded before frontend
    Telemetry telemetry;
    albert::PluginLoader *frontend_plugin;
    albert::Frontend *frontend;
    std::unique_ptr<Session> session;

    AppQueryHandler app_query_handler;
    PluginQueryHandler plugin_query_handler;

    static App *instance();

    void setFrontend(const QString &id);

private:
    void loadAnyFrontend();
    QString loadFrontend(albert::PluginLoader *loader);
    void notifyVersionChange();

    friend int ::main(int, char**);
};
