// Copyright (C) 2025-2025 Manuel Schneider

#include "app.h"
#include "frontend.h"
#include "systemtrayicon.h"
#include "systemutil.h"
#include <QMenu>
#include <QSettings>
#include <QSystemTrayIcon>
static const bool  DEF_SHOWTRAY = true;
static const char* CFG_SHOWTRAY = "showTray";
using namespace albert;
using namespace std;

SystemTrayIcon::SystemTrayIcon(QSettings &s, detail::Frontend &f) : frontend(f)
{
    if (s.value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool())
        setEnabled(true);
}

SystemTrayIcon::~SystemTrayIcon() = default;

bool SystemTrayIcon::isEnabled() const { return tray_icon.get(); }

void SystemTrayIcon::setEnabled(bool enable)
{
    if (enable == isEnabled())
        return;

    else if (enable)
    {
        // menu

        tray_menu = make_unique<QMenu>();

        auto *action = tray_menu->addAction(tr("Show/Hide"));
        QObject::connect(action, &QAction::triggered,
                         [this] { frontend.setVisible(!frontend.isVisible()); });

        action = tray_menu->addAction(tr("Settings"));
        QObject::connect(action, &QAction::triggered,
                         [] { app().showSettings(); });

        action = tray_menu->addAction(tr("Open website"));
        QObject::connect(action, &QAction::triggered,
                         [] { openUrl("https://albertlauncher.github.io/"); });

        tray_menu->addSeparator();

        action = tray_menu->addAction(tr("Restart"));
        QObject::connect(action, &QAction::triggered,
                         [] { App::restart(); });

        action = tray_menu->addAction(tr("Quit"));
        QObject::connect(action, &QAction::triggered,
                         [] { App::quit(); });

        // icon

        auto icon = QIcon::fromTheme("albert-tray");
        icon.setIsMask(true);

        tray_icon = make_unique<QSystemTrayIcon>();
        tray_icon->setIcon(icon);
        tray_icon->setContextMenu(tray_menu.get());
        tray_icon->setVisible(true);

#ifndef Q_OS_MAC
        // Some systems open menus on right click, show albert on left trigger
        QObject::connect(tray_icon.get(), &QSystemTrayIcon::activated,
                         tray_icon.get(), [this](QSystemTrayIcon::ActivationReason reason)
                {
                    if(reason == QSystemTrayIcon::ActivationReason::Trigger)
                        frontend.setVisible(!frontend.isVisible());
                });
#endif
    }
    else
    {
        tray_icon->setVisible(false);
        tray_icon.reset();
        tray_menu.reset();
    }

    app().settings()->setValue(CFG_SHOWTRAY, enable);
}
