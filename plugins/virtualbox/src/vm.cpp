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

#include "vm.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <nsString.h>
#include <nsIServiceManager.h>
#include "util/standardaction.h"
using Core::StandardAction;


/** ***************************************************************************/
VirtualBox::VM::VM(IMachine *machine) : machine_(machine) {
    PRBool isAccessible = PR_FALSE;
    machine->GetAccessible(&isAccessible);

    if (isAccessible) {
        nsXPIDLString machineName;
        machine->GetName(getter_Copies(machineName));
        char *machineNameAscii = ToNewCString(machineName);
        name_ = machineNameAscii;
        free(machineNameAscii);
    } else {
        name_ = "<inaccessible>";
    }

    nsXPIDLString iid;
    machine->GetId(getter_Copies(iid));
    const char *uuidString = ToNewCString(iid);
    uuid_ = uuidString;
    free((void*)uuidString);
}



/** ***************************************************************************/
VirtualBox::VM::~VM() {
    /* don't forget to release the objects in the array... */
    machine_->Release();
}



/*

  A state diagram of the VBox VMStates

            +---------[powerDown()] <- Stuck <--[failure]-+
            V                                             |
    +-> PoweredOff --+-->[powerUp()]--> Starting --+      | +-----[resume()]-----+
    |                |                             |      | V                    |
    |   Aborted -----+                             +--> Running --[pause()]--> Paused
    |                                              |      ^ |                   ^ |
    |   Saved -----------[powerUp()]--> Restoring -+      | |                   | |
    |     ^                                               | |                   | |
    |     |     +-----------------------------------------+-|-------------------+ +
    |     |     |                                           |                     |
    |     |     +- OnlineSnapshotting <--[takeSnapshot()]<--+---------------------+
    |     |                                                 |                     |
    |     +-------- Saving <--------[saveState()]<----------+---------------------+
    |                                                       |                     |
    +-------------- Stopping -------[powerDown()]<----------+---------------------+

 */
/** ***************************************************************************/
VirtualBox::VMItem *VirtualBox::VM::produceItem() const {

    QString pauseCmd = "VBoxManage controlvm %1 pause";
    QString startCmd = "VBoxManage startvm %1";
    QString saveCmd = "VBoxManage controlvm %1 savestate";
    QString stopCmd = "VBoxManage controlvm %1 poweroff";
    QString resetCmd = "VBoxManage controlvm %1 reset";
    QString resumeCmd = "VBoxManage controlvm %1 resume";
    pauseCmd = pauseCmd.arg(uuid_);
    startCmd = startCmd.arg(uuid_);
    saveCmd = saveCmd.arg(uuid_);
    stopCmd = stopCmd.arg(uuid_);
    resetCmd = resetCmd.arg(uuid_);
    resumeCmd = resumeCmd.arg(uuid_);
    ActionSPtrVec actions;
    int mainAction = 0;

    PRUint32 state;
    machine_->GetState(&state);
    switch (state) {
    case MachineState::Starting:
    case MachineState::Restoring:
    case MachineState::Saving:
    case MachineState::Stopping:
        mainAction = VMItem::VM_STATE_CHANGING;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Controls are disabled", [](){}) ));
        break;
    case MachineState::PoweredOff:
    case MachineState::Aborted:
        mainAction = VMItem::VM_START;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Start", [startCmd](){ QProcess::startDetached(startCmd); }) ));
        break;
    case MachineState::Saved:
        mainAction = VMItem::VM_START;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Start", [startCmd](){ QProcess::startDetached(startCmd); }) ));
        break;
    case MachineState::Running:
        mainAction = VMItem::VM_PAUSE;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Pause", [pauseCmd](){ QProcess::startDetached(pauseCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Save State", [saveCmd](){ QProcess::startDetached(saveCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Stop", [stopCmd](){ QProcess::startDetached(stopCmd); }) ));
        break;
    case MachineState::Paused:
        mainAction = VMItem::VM_RESUME;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Resume", [resumeCmd](){ QProcess::startDetached(resumeCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Save State", [saveCmd](){ QProcess::startDetached(saveCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Reset", [resetCmd](){ QProcess::startDetached(resetCmd); }) ));
        break;
    default:
        mainAction = VMItem::VM_DIFFERENT;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Controls are disabled", [](){}) ));
    }

    return new VMItem(name_, uuid_, mainAction, actions, state_);
}



/** ***************************************************************************/
bool VirtualBox::VM::startsWith(QString other) const {
    return name_.startsWith(other, Qt::CaseInsensitive);
}



/** ***************************************************************************
void VirtualBox::VM::probeState() const {
    QProcess *process = new QProcess;
    process->setReadChannel(QProcess::StandardOutput);
    process->start("VBoxManage",  {"showvminfo", uuid_, "--machinereadable"});
    QObject::connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        [this, process](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitStatus == QProcess::NormalExit && exitCode == 0){
            while (process->canReadLine()) {
               QString line = QString::fromLocal8Bit(process->readLine());
               if (line.startsWith("VMState=")) {
                   QRegularExpression regex("VMState=\"(.*)\"");
                   QRegularExpressionMatch match = regex.match(line);
                   state_ = match.captured(1).toLower();
                   break;
               }
            }
        }
        process->deleteLater();
    });
    QObject::connect(process, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), [process](QProcess::ProcessError){
        process->deleteLater();
    });
}*/

