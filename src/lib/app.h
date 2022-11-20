// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensionregistry.h"
#include "albert/extensions/indexqueryhandler.h"
#include "pluginprovider.h"
#include "queryengine.h"
#include "rpcserver.h"
#include "settings/settingswindow.h"
#include "terminalprovider.h"
#include "usagehistory.h"

class App : public albert::IndexQueryHandler
{
public:
    explicit App(const QStringList &additional_plugin_paths);

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
    QPointer<SettingsWindow> settings_window;
};

