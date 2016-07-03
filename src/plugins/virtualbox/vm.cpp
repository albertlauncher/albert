#include "vm.h"

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include "standardobjects.h"

VirtualBox::VM::VM(QString &listVmsLine) {
    QRegularExpression regex("\"(.*)\" {(.*)}");
    QRegularExpressionMatch match = regex.match(listVmsLine);
    name_ = match.captured(1);
    uuid_ = match.captured(2);

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

VirtualBox::VMItem *VirtualBox::VM::produceItem() {
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
    if (state_ == "starting" || state_ == "restoring" || state_ == "saving" || state_ == "stopping")
        mainAction = VMItem::VM_STATE_CHANGING;
    else if (state_ == "poweroff" || state_ == "aborted")
        mainAction = VMItem::VM_START;
    else if (state_ == "saved")
        mainAction = VMItem::VM_START;
    else if (state_ == "running") {
        mainAction = VMItem::VM_PAUSE;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Pause", [pauseCmd](ExecutionFlags*){ QProcess::startDetached(pauseCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Save State", [saveCmd](ExecutionFlags*){ QProcess::startDetached(saveCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Stop", [stopCmd](ExecutionFlags*){ QProcess::startDetached(stopCmd); }) ));
    } else if (state_ == "paused") {
        mainAction = VMItem::VM_RESUME;
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Resume", [resumeCmd](ExecutionFlags*){ QProcess::startDetached(resumeCmd); }) ));
        actions.push_back(std::shared_ptr<StandardAction>( new StandardAction("Reset", [resetCmd](ExecutionFlags*){ QProcess::startDetached(resetCmd); }) ));
    }

    return new VMItem(name_, uuid_, mainAction, actions, state_);
}



bool VirtualBox::VM::startsWith(QString other) {
    return name_.startsWith(other, Qt::CaseInsensitive);
}

