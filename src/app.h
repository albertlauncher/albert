// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extensionregistry.h"
#include "appqueryhandler.h"
#include "pluginconfigqueryhandler.h"
#include "pluginqueryhandler.h"
#include "pluginregistry.h"
#include "qtpluginprovider.h"
#include "queryengine.h"
#include "rpcserver.h"
#include "telemetry.h"
#include "terminalprovider.h"
#include <QObject>
#include <QPointer>
class PluginsWidget;
class QHotkey;
class Session;
class SettingsWindow;
class TrayIcon;
namespace albert {
class PluginLoader;
class Frontend;
}

extern int run(int, char**);

class App : public QObject
{
    Q_OBJECT

public:

    QStringList availableFrontends();
    QString currentFrontend();
    void setFrontend(uint i);

    albert::Frontend *frontend();

    void showSettings(QString plugin_id = {});

    bool trayEnabled() const;
    void setTrayEnabled(bool);

    const QHotkey *hotkey() const;
    void setHotkey(std::unique_ptr<QHotkey> hotkey);

    TerminalProvider &terminal();

    PluginsWidget *makePluginsWidget();
    QWidget *makeQueryWidget();

private:

    explicit App(const QStringList &additional_plugin_paths, bool load_enabled);
    ~App();

    void initialize();
    void finalize();

    static App *instance();

    void initHotkey();
    void loadAnyFrontend();
    QString loadFrontend(albert::PluginLoader *loader);
    void notifyVersionChange();

    RPCServer rpc_server; // Check for other instances first
    albert::ExtensionRegistry extension_registry_;
    PluginRegistry plugin_registry_;
    QueryEngine query_engine_;
    QtPluginProvider plugin_provider;
    TerminalProvider terminal_provider_;
    QPointer<SettingsWindow> settings_window;
    std::unique_ptr<QHotkey> hotkey_;
    Telemetry telemetry;
    albert::PluginLoader *frontend_plugin;
    albert::Frontend *frontend_;
    AppQueryHandler app_query_handler;
    PluginQueryHandler plugin_query_handler;
    PluginConfigQueryHandler plugin_config_query_handler;
    std::unique_ptr<Session> session;
    std::unique_ptr<TrayIcon> tray_icon;

    friend int ::run(int, char**);
};
