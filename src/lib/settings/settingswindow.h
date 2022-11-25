// Copyright (c) 2014-2022 Manuel Schneider
#pragma once
#include "ui_settingswindow.h"
#include <QWidget>
namespace albert { class ExtensionRegistry; }
class App;
class NativePluginProvider;
class TerminalProvider;
class QueryEngine;



class SettingsWindow final : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(App &app);

    void bringToFront();

    Ui::SettingsWindow ui;

private:
    void init_tab_general_frontend(NativePluginProvider&);
    void init_tab_general_terminal(TerminalProvider&);
    void init_tab_general_trayIcon();
    void init_tab_general_autostart();
    void init_tab_general_search(QueryEngine&);
    void init_tab_about();

    void keyPressEvent(QKeyEvent * event) override;

//    void closeEvent(QCloseEvent * event) override;
//    void init_tab_general_hotkey();
//  void init_autostart();
//    void changeHotkey(int);
};
