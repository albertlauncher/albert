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

#include "item.h"
#include "standardaction.h"
using Core::StandardAction;
#include <QDBusConnection>


/** ***************************************************************************/
MPRIS::Item::Item(Player &p, const QString &subtext, const QString &iconPath, const QDBusMessage &msg, bool hideAfter)
    : player_(p), subtext_(subtext), iconPath_(iconPath), message_(msg), hideAfter_(hideAfter) {
    text_ = p.getName();
    actions_.push_back(shared_ptr<Action>(new StandardAction(subtext, [this](){
        QDBusConnection::sessionBus().send(message_);
//        flags->hideWidget = hideAfter_;
//        flags->clearInput = hideAfter_;
    })));
    if (p.canRaise()) {
        actions_.push_back(shared_ptr<Action>(new StandardAction("Raise Window", [&p](){
            QString busid = p.getBusId();
            QDBusMessage raise = QDBusMessage::createMethodCall(busid, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Raise");
            if (!QDBusConnection::sessionBus().send(raise)) {
                qWarning("Error calling raise method on dbus://%s", busid.toStdString().c_str());
            }
        })));
    }
    id_ = "extension.mpris.item:%1.%2";
    id_ = id_.arg(p.getBusId()).arg(msg.member());
}



/** ***************************************************************************/
MPRIS::Item::~Item() {

}



/** ***************************************************************************/
vector<shared_ptr<Action>> MPRIS::Item::actions() {
    return actions_;
}

