// Copyright (c) 2014-2022 Manuel Schneider
#pragma once
#include "ui_settingswindow.h"
#include <QWidget>
class App;

class SettingsWindow final : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWindow(App &app);
    void bringToFront();

private:
    void init_tabs(App &app);

    void init_tab_general_hotkey(App &app);
    void init_tab_general_trayIcon(App &app);
    void init_tab_general_autostart();
    void init_tab_general_frontends(App &app);
    void init_tab_general_terminals(App &app);
    void init_tab_general_search(App &app);
    void init_tab_about();

    void keyPressEvent(QKeyEvent * event) override;

    Ui::SettingsWindow ui;
};
