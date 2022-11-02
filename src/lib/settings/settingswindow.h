// Copyright (c) 2014-2022 Manuel Schneider
#pragma once
#include "ui_settingswindow.h"
#include <QWidget>
namespace albert { class ExtensionRegistry; }
class FrontendProvider;
class PluginProvider;
class TerminalProvider;

class SettingsWindow final : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(albert::ExtensionRegistry&,
                            PluginProvider&,
                            TerminalProvider&);

    void bringToFront();

    Ui::SettingsWindow ui;

private:
    void init_tab_general_about();
    void init_tab_general_frontend(PluginProvider&);
    void init_tab_general_terminal(TerminalProvider&);
    void init_tab_settings(albert::ExtensionRegistry&);
    void init_tab_plugins(albert::ExtensionRegistry&);

    void keyPressEvent(QKeyEvent * event) override;

//    void init_tab_general_trayIcon();
//    void closeEvent(QCloseEvent * event) override;
//    void init_tab_general_hotkey();
//  void init_autostart();
//    void changeHotkey(int);
};
