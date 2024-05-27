// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/util/iconprovider.h"
#include "trayicon.h"


TrayIcon::TrayIcon()
{
    auto icon = albert::iconFromUrls({"xdg:albert-tray", "xdg:albert", ":app_tray_icon"});
    icon.setIsMask(true);
    setIcon(icon);

#ifndef Q_OS_MAC
    // Some systems open menus on right click, show albert on left trigger
    QObject::connect(this, &TrayIcon::activated, [](QSystemTrayIcon::ActivationReason reason){
        if( reason == QSystemTrayIcon::ActivationReason::Trigger)
            albert::toggle();
    });
#endif

    auto *action = menu.addAction(tr("Show/Hide"));
    QObject::connect(action, &QAction::triggered, [](){ albert::toggle(); });

    action = menu.addAction(tr("Settings"));
    QObject::connect(action, &QAction::triggered, [](){ albert::showSettings(); });

    action = menu.addAction(tr("Open website"));
    QObject::connect(action, &QAction::triggered, [](){ albert::openWebsite(); });

    menu.addSeparator();

    action = menu.addAction(tr("Restart"));
    QObject::connect(action, &QAction::triggered, [](){ albert::restart(); });

    action = menu.addAction(tr("Quit"));
    QObject::connect(action, &QAction::triggered, [](){ albert::quit(); });

    setContextMenu(&menu);
    setVisible(true);
}
