// albert - a simple application launcher for linux
// Copyright (C) 2016-2017 Martin Buergmann
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

#include "vmitem.h"

#include <QProcess>

/** ***************************************************************************/
QString VirtualBox::VMItem::iconPath_;
const int VirtualBox::VMItem::VM_START = 1;
const int VirtualBox::VMItem::VM_PAUSE = 2;
const int VirtualBox::VMItem::VM_RESUME = 3;
const int VirtualBox::VMItem::VM_STATE_CHANGING = -1;

VirtualBox::VMItem::VMItem(const QString &name, const QString &uuid, int &mainAction, const ActionSPtrVec actions, const QString &state) : name_(name), uuid_(uuid), actions_(actions), mainAction_(mainAction) {
    idstring_ = QString("extension.virtualbox.item:%1.%2").arg(uuid).arg(state);
}

QString VirtualBox::VMItem::subtext() const {
    QString toreturn;
    switch (mainAction_) {
    case VM_START:
        toreturn = "Start %1";
        break;
    case VM_PAUSE:
        toreturn = "Pause %1";
        break;
    case VM_RESUME:
        toreturn = "Resume %1";
        break;
    case VM_STATE_CHANGING:
        toreturn = "The VM %1 is currently in action. Controls are disabled!";
        break;
    default:
        toreturn = "Start %1";
        break;
    }
    return toreturn.arg(name_);
}

/*
void VirtualBox::VMItem::activate(Action::ExecutionFlags *) {
    QString executionCommand;
    switch (mainAction_) {
    case VM_START:
        executionCommand = "vboxmanage startvm %1";
        break;
    case VM_PAUSE:
        executionCommand = "vboxmanage controlvm %1 pause";
        break;
    case VM_RESUME:
        executionCommand = "vboxmanage controlvm %1 resume";
        break;
    case VM_STATE_CHANGING:
        break;
    default:
        executionCommand = "vboxmanage startvm %1";
        break;
    }
    if (!executionCommand.isEmpty())
        QProcess::startDetached(executionCommand.arg(uuid_));
}
*/
