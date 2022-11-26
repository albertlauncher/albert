// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensionregistry.h"
#include "albert/extensions/indexqueryhandler.h"
#include "pluginprovider.h"
#include "queryengine.h"
#include "rpcserver.h"
#include "settings/settingswindow.h"
#include "terminalprovider.h"
#include "trayicon.h"
#include "usagedatabase.h"
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
    std::vector<albert::IndexItem> indexItems() const override;

public:

    RPCServer rpc_server;
    albert::ExtensionRegistry extension_registry;
    QueryEngine query_engine;
    NativePluginProvider plugin_provider;
    TerminalProvider terminal_provider;
    TrayIcon tray_icon;
    QPointer<SettingsWindow> settings_window;
};

