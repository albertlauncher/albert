// Copyright (c) 2022 Manuel Schneider

#include <QApplication>
#include <QSettings>
#include "trayicon.h"
#include "albert/util/util.h"
#include "albert/albert.h"
#include "xdg/iconlookup.h"

namespace {
    const char* CFG_SHOWTRAY = "showTray";
    const bool  DEF_SHOWTRAY = true;
}

TrayIcon::TrayIcon() {

    if (auto icon = XDG::IconLookup::iconPath({"albert-tray", "albert"}); icon.isNull())
        setIcon(qApp->windowIcon());
    else
        setIcon(QPixmap(icon)); // https://bugreports.qt.io/browse/QTBUG-53550

    setVisible(QSettings().value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool());

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
    QSettings().setValue(CFG_SHOWTRAY, enable);
    QSystemTrayIcon::setVisible(enable);
}

