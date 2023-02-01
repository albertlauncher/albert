// Copyright (c) 2014-2022 Manuel Schneider
#pragma once
#include "ui_settingswindow.h"
#include <QWidget>
namespace albert { class ExtensionRegistry; }
class App;
class NativePluginProvider;
class TerminalProvider;
class QueryEngine;
class TrayIcon;
class Hotkey;



class SettingsWindow final : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(App &app);

    void bringToFront();

private:
    void init_tab_general_hotkey(Hotkey&);
    void init_tab_general_frontend(NativePluginProvider&);
    void init_tab_general_terminal(TerminalProvider&);
    void init_tab_general_trayIcon(TrayIcon&);
    void init_tab_general_autostart();
    void init_tab_general_search(QueryEngine&);
    void init_tab_about();

    void keyPressEvent(QKeyEvent * event) override;

    Ui::SettingsWindow ui;
};
