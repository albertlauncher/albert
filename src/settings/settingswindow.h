// Copyright (c) 2014-2024 Manuel Schneider

#pragma once
#include "ui_settingswindow.h"
#include <QWidget>
class Application;
class PluginsWidget;

class SettingsWindow final : public QWidget
{
    Q_OBJECT

public:

    SettingsWindow(Application &app);
    ~SettingsWindow();

    void bringToFront(const QString & = {});

private:

    void init_tab_general_hotkey();
    void init_tab_general_frontends();
    void init_tab_general_path();
    void init_tab_general_trayIcon();
    void init_tab_general_telemetry();
    void init_tab_general_about();
    void keyPressEvent(QKeyEvent * event) override;

    Application &app;
    Ui::SettingsWindow ui;
    PluginsWidget *plugin_widget;
    QString small_text_fmt;
};
