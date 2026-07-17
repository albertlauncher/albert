// Copyright (C) 2025-2025 Manuel Schneider

#pragma once
#include <QCoreApplication>
#include <memory>
class QMenu;
class QSettings;
class QSystemTrayIcon;
namespace albert::detail { class Frontend; }

class SystemTrayIcon
{
    Q_DECLARE_TR_FUNCTIONS(SystemTrayIcon)

public:
    SystemTrayIcon(QSettings &settings,
                   albert::detail::Frontend &frontend);
    ~SystemTrayIcon();

    bool isEnabled() const;
    void setEnabled(bool);

private:
    albert::detail::Frontend &frontend;
    std::unique_ptr<QSystemTrayIcon> tray_icon;
    std::unique_ptr<QMenu> tray_menu;
};
