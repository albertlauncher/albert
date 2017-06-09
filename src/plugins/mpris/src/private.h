// albert extension mpris - a mpris interface plugin for albert
// Copyright (C) 2017 Martin Buergmann
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

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QStringList>
#include <QMap>
#include <QPointer>


namespace MPRIS {

class ConfigWidget;
class Player;
class Command;

class MPRISPrivate {
public:
    ~MPRISPrivate();


    const char* name = "MPRIS Control";
    static QDBusMessage findPlayerMsg;
    QPointer<MPRIS::ConfigWidget> widget;
    QList<MPRIS::Player*> mediaPlayers;
    QStringList commands;
    QMap<QString, MPRIS::Command> commandObjects;

    static const int DBUS_TIMEOUT = 25 /* ms */;

    QDBusMessage call(QDBusMessage &toDispatch);

};

} // namespace MPRIS
