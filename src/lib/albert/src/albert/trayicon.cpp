// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QSettings>
#include "trayicon.h"

namespace {
    const char* CFG_SHOWTRAY = "showTray";
    const bool  DEF_SHOWTRAY = true;
}

/** ***************************************************************************/
TrayIcon::TrayIcon() {
    setIcon(qApp->windowIcon());
    if (QSettings(qApp->applicationName()).value(CFG_SHOWTRAY, DEF_SHOWTRAY).toBool())
        setVisible(true);
}


/** ***************************************************************************/
void TrayIcon::setVisible(bool enable) {
    QSettings(qApp->applicationName()).setValue(CFG_SHOWTRAY, enable);
    QSystemTrayIcon::setVisible(enable);
    emit stateChanged(enable);
}
