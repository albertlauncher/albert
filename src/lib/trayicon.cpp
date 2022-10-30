// Copyright (c) 2022 Manuel Schneider

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
    // https://bugreports.qt.io/browse/QTBUG-53550
    setIcon(QPixmap(XDG::IconLookup::iconPath({"albert-tray", "albert"})));
    if (QSettings().value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool())
        setVisible(true);
}


/** ***************************************************************************/
void Core::TrayIcon::setVisible(bool enable) {
    QSettings().setValue(CFG_SHOWTRAY, enable);
    QSystemTrayIcon::setVisible(enable);
    emit stateChanged(enable);
}
