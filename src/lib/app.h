// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "extension.h"
#include "extensionregistry.h"
#include "frontend.h"
#include "logging.h"
#include "pluginprovider.h"
#include "queryengine.h"
#include "rpcserver.h"
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QObject>
#include <QSettings>
#include <QString>
#include <map>

class App final
{
public:
    App(const QStringList &additional_plugin_dirs);

    static App *instance();

    void showSettings();

    albert::ExtensionRegistry extension_registry;
    PluginProvider plugin_provider;
    QueryEngine *query_engine;
    albert::Frontend *frontend;

private:
    void notifyVersionChangeAndFirstRun();
    void loadFrontend();
    QWidget *createSettingsWindow();

//    RPCServer rpc_server;
//    QPointer<SettingsWindow> settings_window;
//    QString handleSocketMessage(const QString &message) override;
//    TrayIcon tray_icon;
//    TerminalProvider terminal_provider;
};
