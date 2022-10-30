// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "extension.h"
#include "extensionregistry.h"
#include "frontend.h"
#include "logging.h"
#include "pluginprovider.h"
#include "queryengine.h"
#include "terminalprovider.h"
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
    TerminalProvider terminal_provider;
    PluginProvider plugin_provider;
    QueryEngine *query_engine;
    RPCServer rpc_server;
    albert::Frontend *frontend;
//    QPointer<SettingsWindow> settings_window;

private:
    void notifyVersionChangeAndFirstRun();
    void loadFrontend();
    QWidget *createSettingsWindow();
    QString handleSocketMessage(const QString &message);
    TerminalProvider terminal;
//    TrayIcon tray_icon;
};
