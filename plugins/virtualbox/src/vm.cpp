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
#include "util/standardaction.h"
using Core::StandardAction;


/** ***************************************************************************/
VirtualBox::VM::VM(const QString vboxFileName) {

    QFile vboxFile(vboxFileName);
    if (!vboxFile.open(QFile::ReadOnly)) {
        qWarning("Could not open VM config file for read operation: %s", vboxFileName.toStdString().c_str());
        return;
    }

    QDomDocument machineConfig;
    QString errMsg;
    int errLine, errCol;
    if (!machineConfig.setContent(&vboxFile, &errMsg, &errLine, &errCol)) {
        qWarning("Could not parse VM config file %s because %s in line %d col %d", vboxFileName.toStdString().c_str(), errMsg.toStdString().c_str(), errLine, errCol);
        state_ = "";
        vboxFile.close();
        return;
    }
    vboxFile.close();

    QDomElement root = machineConfig.documentElement();
    QDomElement machine = root.firstChildElement("Machine");
    uuid_ = machine.attribute("uuid");
    name_ = machine.attribute("name");
    state_ = "poweroff";

    probeState();
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
    if (state_.isEmpty())
        return nullptr; // This should not be empty... We just ignore this VM

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
    if (state_ == "starting" || state_ == "restoring" || state_ == "saving" || state_ == "stopping") {
        mainAction = VMItem::VM_STATE_CHANGING;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Controls are disabled", [](){}) ));
    } else if (state_ == "poweroff" || state_ == "aborted") {
        mainAction = VMItem::VM_START;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Start", [startCmd](){ QProcess::startDetached(startCmd); }) ));
    } else if (state_ == "saved") {
        mainAction = VMItem::VM_START;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Start", [startCmd](){ QProcess::startDetached(startCmd); }) ));
    } else if (state_ == "running") {
        mainAction = VMItem::VM_PAUSE;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Pause", [pauseCmd](){ QProcess::startDetached(pauseCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Save State", [saveCmd](){ QProcess::startDetached(saveCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Stop", [stopCmd](){ QProcess::startDetached(stopCmd); }) ));
    } else if (state_ == "paused") {
        mainAction = VMItem::VM_RESUME;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Resume", [resumeCmd](){ QProcess::startDetached(resumeCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Save State", [saveCmd](){ QProcess::startDetached(saveCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Reset", [resetCmd](){ QProcess::startDetached(resetCmd); }) ));
    }

    return new VMItem(name_, uuid_, mainAction, actions, state_);
}



/** ***************************************************************************/
bool VirtualBox::VM::startsWith(QString other) const {
    return name_.startsWith(other, Qt::CaseInsensitive);
}



/** ***************************************************************************/
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
}

