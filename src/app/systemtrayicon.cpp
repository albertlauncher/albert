// Copyright (C) 2025-2025 Manuel Schneider

#include "application.h"
#include "systemutil.h"
#include "systemtrayicon.h"
#include <QCoreApplication>
#include <QMenu>
#include <QSettings>
#include <QSystemTrayIcon>
static const bool  DEF_SHOWTRAY = true;
static const char* CFG_SHOWTRAY = "showTray";
using namespace albert;
using namespace std;

static inline QString tr(const char *sourceText, const char *disambiguation = nullptr, int n = -1)
{ return QCoreApplication::translate("SystemTrayIcon", sourceText, disambiguation, n); }


SystemTrayIcon::SystemTrayIcon(QSettings &settings)
{
    if (settings.value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool())
        setEnabled(true);
}

SystemTrayIcon::~SystemTrayIcon() = default;

bool SystemTrayIcon::isEnabled() const
{
    return tray_icon.get();
}

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
                         [] { Application::instance().toggle(); });

        action = tray_menu->addAction(tr("Settings"));
        QObject::connect(action, &QAction::triggered,
                         [] { Application::instance().showSettings(); });

        action = tray_menu->addAction(tr("Open website"));
        QObject::connect(action, &QAction::triggered,
                         [] { openUrl("https://albertlauncher.github.io/"); });

        tray_menu->addSeparator();

        action = tray_menu->addAction(tr("Restart"));
        QObject::connect(action, &QAction::triggered,
                         [] { Application::restart(); });

        action = tray_menu->addAction(tr("Quit"));
        QObject::connect(action, &QAction::triggered,
                         [] { Application::quit(); });

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
                         tray_icon.get(), [](QSystemTrayIcon::ActivationReason reason)
                {
                    if(reason == QSystemTrayIcon::ActivationReason::Trigger)
                        Application::instance().toggle();
                });
#endif
    }
    else
    {
        tray_icon->setVisible(false);
        tray_icon.reset();
        tray_menu.reset();
    }

    App::settings()->setValue(CFG_SHOWTRAY, enable);
}
