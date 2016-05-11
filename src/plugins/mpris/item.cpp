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
#include <QDBusConnection>
#include "albertapp.h"


/** ***************************************************************************/
MPRIS::Item::Item(Player &p, QString &subtext, QString &iconPath, QDBusMessage &msg, bool hideAfter)
    : player_(p), subtext_(subtext), iconPath_(iconPath), message_(msg), hideAfter_(hideAfter)
{
    text_ = p.getName();
}

MPRIS::Item::~Item(){

}



/** ***************************************************************************/
QString MPRIS::Item::text() const {
    return text_;
}



/** ***************************************************************************/
QString MPRIS::Item::subtext() const {
    return subtext_;
}



/** ***************************************************************************/
QString MPRIS::Item::iconPath() const {
    return iconPath_;
}



/** ***************************************************************************/
void MPRIS::Item::activate(ExecutionFlags *flags) {
    QDBusConnection::sessionBus().send(message_);
    flags->hideWidget = hideAfter_;
    flags->clearInput = hideAfter_;
//    if (hideAfter_)
//        qApp->hideWidget();
}



/** ***************************************************************************/
bool MPRIS::Item::hasChildren() const {
    // Performance measure.
    return false;
}



/** ***************************************************************************/
vector<shared_ptr<AlbertItem>> MPRIS::Item::children() {
    // Return the children.
    // Did not want to have children? Subclass A2leaf instead.
    return vector<shared_ptr<AlbertItem>>();
}

