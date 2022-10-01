// Copyright (C) 2014-2018 Manuel Schneider

#include <QApplication>
#include <QSettings>
#include "trayicon.h"
#include "xdg/iconlookup.h"

namespace {
    const char* CFG_SHOWTRAY = "showTray";
    const bool  DEF_SHOWTRAY = true;
}

/** ***************************************************************************/
Core::TrayIcon::TrayIcon() {
    setIcon(QIcon(XDG::IconLookup::iconPath({"albert-tray", "albert"})));
    if (QSettings(qApp->applicationName()).value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool())
        setVisible(true);
}


/** ***************************************************************************/
void Core::TrayIcon::setVisible(bool enable) {
    QSettings(qApp->applicationName()).setValue(CFG_SHOWTRAY, enable);
    QSystemTrayIcon::setVisible(enable);
    emit stateChanged(enable);
}
