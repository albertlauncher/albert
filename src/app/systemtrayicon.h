// Copyright (C) 2025-2025 Manuel Schneider

#pragma once
#include <memory>
class QSettings;
class QSystemTrayIcon;
class QMenu;

class SystemTrayIcon
{
public:
    SystemTrayIcon(QSettings &settings);
    ~SystemTrayIcon();

    bool isEnabled() const;
    void setEnabled(bool);

private:
    std::unique_ptr<QSystemTrayIcon> tray_icon;
    std::unique_ptr<QMenu> tray_menu;
};
