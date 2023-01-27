// Copyright (c) 2022-2023 Manuel Schneider

#include <QApplication>
#include <QSettings>
#include "trayicon.h"
#include "albert/util/util.h"
#include "albert/albert.h"
#include "xdg/iconlookup.h"
#include "albert/logging.h"

namespace {
    const char* CFG_SHOWTRAY = "showTray";
    const bool  DEF_SHOWTRAY = true;
}

TrayIcon::TrayIcon() {

    if (!supportsMessages())
        WARN << "Desktop notifications are not supported on this system";

    // https://bugreports.qt.io/browse/QTBUG-53550
    QPixmap pm = XDG::IconLookup::iconPath("albert-tray");
    if (pm.isNull())
        pm = QPixmap(":app_tray_icon");
    QIcon icon(pm);
    icon.setIsMask(true);
    setIcon(icon);

    setVisible(QSettings(qApp->applicationName()).value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool());

//    QObject::connect(this, &TrayIcon::activated, [](QSystemTrayIcon::ActivationReason reason){
//        if( reason == QSystemTrayIcon::ActivationReason::Trigger)
//            albert::toggle();
//    });

    auto *action = menu.addAction("Show/Hide");
    QObject::connect(action, &QAction::triggered, [](){ albert::toggle(); });

    action = menu.addAction("Settings");
    QObject::connect(action, &QAction::triggered, [](){ albert::showSettings(); });

    action = menu.addAction("Open website");
    QObject::connect(action, &QAction::triggered, [](){ albert::openWebsite(); });

    menu.addSeparator();

    action = menu.addAction("Restart");
    QObject::connect(action, &QAction::triggered, [](){ albert::restart(); });

    action = menu.addAction("Quit");
    QObject::connect(action, &QAction::triggered, [](){ albert::quit(); });

    setContextMenu(&menu);
}

void TrayIcon::setVisible(bool enable) {
    QSettings(qApp->applicationName()).setValue(CFG_SHOWTRAY, enable);
    QSystemTrayIcon::setVisible(enable);
}

