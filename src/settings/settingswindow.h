// Copyright (c) 2014-2026 Manuel Schneider

#pragma once
#include "ui_settingswindow.h"
#include <QWidget>
class FrontendRegistry;
class HotkeyManager;
class Localization;
class PathManager;
class PluginRegistry;
class PluginsWidget;
class QueryEngine;
class SystemTrayIcon;
class Telemetry;

class SettingsWindow final : public QWidget
{
    Q_OBJECT

public:
    SettingsWindow(FrontendRegistry &,
                   HotkeyManager &,
                   Localization &,
                   PathManager &,
                   PluginRegistry &,
                   QueryEngine &,
                   SystemTrayIcon &,
                   Telemetry &);
    ~SettingsWindow();

    void bringToFront(const QString & = {});

private:

    void init_tab_general_hotkey(HotkeyManager &);
    void init_tab_general_frontends(FrontendRegistry &);
    void init_tab_general_path(PathManager &);
    void init_tab_general_trayIcon(SystemTrayIcon &);
    void init_tab_general_telemetry(Telemetry &);
    void init_tab_general_localization(Localization &);
    void init_tab_general_about();
    void keyPressEvent(QKeyEvent * event) override;

    Ui::SettingsWindow ui;
    PluginsWidget *plugin_widget;
    QString small_text_fmt;
};
