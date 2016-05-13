// albert extension mpris - a mpris interface plugin for albert
// Copyright (C) 2016 Martin Buergmann
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

#include "player.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>

/** ***************************************************************************/
MPRIS::Player::Player(QString &busid) : busid_(busid), name_(busid) {
    // Query the name of the media player of which we have the bus id.
    QDBusInterface iface(busid, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2");
    QVariant prop = iface.property("Identity");
    QString name = prop.toString();
    if (!name.isNull() && !name.isEmpty()) {
        name_ = name;
    } else {
        qWarning("DBus: Name is either empty or null");
    }
}



/** ***************************************************************************/
QString& MPRIS::Player::getBusId() {
    return busid_;
}



/** ***************************************************************************/
QString& MPRIS::Player::getName() {
    return name_;
}
