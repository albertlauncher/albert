// Copyright (c) 2014-2022 Manuel Schneider
#pragma once
#include "ui_settingswindow.h"
#include <QWidget>
class App;

class SettingsWindow final : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(App &albert);

    void bringToFront();

private:

    void keyPressEvent(QKeyEvent * event) override;
    void closeEvent(QCloseEvent * event) override;

    void init_tab_general_hotkey();
    void init_tab_general_frontend();
    void init_tab_general_terminal();
    void init_tab_general_trayIcon();
    void init_tab_general_about();
//  void init_autostart();

    void changeHotkey(int);

    App &app;
    Ui::SettingsWindow ui;
};
