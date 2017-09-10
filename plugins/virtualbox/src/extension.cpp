// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
// Contributed to by 2016-2017 Martin Buergmann
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

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QFileSystemWatcher>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QListWidget>
#include <functional>
#include "extension.h"
#include "vm.h"
#include "core/query.h"
#include "util/standarditem.h"
#include "util/standardaction.h"
#include "xdg/iconlookup.h"
using Core::Action;
using Core::StandardAction;
using Core::StandardItem;

#include <nsString.h>
#include <nsIServiceManager.h>
#include "VirtualBox_XPCOM.h"

#include "configwidget.h"


class VirtualBox::Private
{
public:

    QPointer<ConfigWidget> widget;
    QList<VM*> vms;
    QFileSystemWatcher vboxWatcher;

    nsCOMPtr<nsIComponentManager> manager;
    nsCOMPtr<IVirtualBox> virtualBox;


    //void rescanVBoxConfig(QString path);
    void pollMachines();
    void reload();

};



/** ***************************************************************************
void VirtualBox::Private::rescanVBoxConfig(QString path) {

    qInfo() << "Start indexing VirtualBox images.";

    QFile vboxConfigFile(path);
    if (!vboxConfigFile.exists())
        return;
    if (!vboxConfigFile.open(QFile::ReadOnly)) {
        qCritical() << "Could not open VirtualBox config file for read operation!";
        return;
    }

    QDomDocument vboxConfig;
    QString errMsg = "";
    int errLine = 0, errCol = 0;
    if (!vboxConfig.setContent(&vboxConfigFile, &errMsg, &errLine, &errCol)) {
        qWarning() << qPrintable(QString("Parsing VBox config failed because %s in line %d col %d").arg(errMsg).arg(errLine, errCol));
        vboxConfigFile.close();
        return;
    }
    vboxConfigFile.close();

    QDomElement root = vboxConfig.documentElement();
    if (root.isNull()) {
        qCritical() << "In VBox config file: Root element is null.";
        return;
    }

    QDomElement global = root.firstChildElement("Global");
    if (global.isNull()) {
        qCritical() << "In VBox config file: Global element is null.";
        return;
    }

    QDomElement machines = global.firstChildElement("MachineRegistry");  // List of MachineEntry
    if (machines.isNull()) {
        qCritical() << "In VBox config file: Machine registry element is null.";
        return;
    }

    // With this we iterate over the machine entries
    QDomElement machine = machines.firstChildElement();

    // And we count how many entries we find for information reasons
    int found = 0;

    qDeleteAll(vms);
    vms.clear();

    while (!machine.isNull()) {

        vms.append(new VM(machine.attribute("src")));

        machine = machine.nextSiblingElement();
        found++;
    }

    qInfo() << qPrintable(QString("Indexed %2 VirtualBox images.").arg(found));
}



/** ***************************************************************************/
void VirtualBox::Private::pollMachines() {

    qDeleteAll(vms);
    vms.clear();

    nsresult rc;
    IMachine **machines = NULL;
    PRUint32 machineCnt = 0;

    rc = virtualBox->GetMachines(&machineCnt, &machines);
    if (NS_SUCCEEDED(rc)) {
        for (PRUint32 i = 0; i < machineCnt; ++ i) {
            IMachine *machine = machines[i];
            if (machine) {
                vms.append(new VM(machine));
            }
        }
    }

}



/** ***************************************************************************/
void VirtualBox::Private::reload() {
    pollMachines();
    widget->ui.listWidget->clear();

    for (VM* vm : vms) {
        widget->ui.listWidget->addItem(vm->name());
    }
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
VirtualBox::Extension::Extension()
    : Core::Extension("org.albert.extension.virtualbox"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    nsresult rc;

    rc = NS_GetComponentManager(getter_AddRefs(d->manager));
    if (NS_FAILED(rc))
        throw("Error: could not get component manager.");

    rc = d->manager->CreateInstanceByContractID(NS_VIRTUALBOX_CONTRACTID, nsnull,
                                             NS_GET_IID(IVirtualBox),
                                             getter_AddRefs(d->virtualBox));
    if (NS_FAILED(rc))
        throw("Error, could not instantiate VirtualBox object.");

    VMItem::iconPath_ = XDG::IconLookup::iconPath("virtualbox");
    if ( VMItem::iconPath_.isNull() )
        VMItem::iconPath_ = ":vbox";

    /*QString vboxConfigPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, "VirtualBox/VirtualBox.xml");
    if (vboxConfigPath.isEmpty())
        throw "VirtualBox was not detected!";

    d->rescanVBoxConfig(vboxConfigPath);
    d->vboxWatcher.addPath(vboxConfigPath);
    connect(&d->vboxWatcher, &QFileSystemWatcher::fileChanged,
            std::bind(&Private::rescanVBoxConfig, d.get(), std::placeholders::_1));*/
}



/** ***************************************************************************/
VirtualBox::Extension::~Extension() {
    d->virtualBox = nsnull;
    NS_ShutdownXPCOM(nsnull);
}



/** ***************************************************************************/
QWidget *VirtualBox::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        d->reload();

        connect(d->widget->ui.pushButton, &QPushButton::clicked, [=]() {d->reload();});
    }
    return d->widget;
}



/** ***************************************************************************/
void VirtualBox::Extension::setupSession() {
    d->pollMachines();
}



/** ***************************************************************************/
void VirtualBox::Extension::handleQuery(Core::Query * query) const {

    if ( query->searchTerm().isEmpty() )
        return;

    for (VM* vm : d->vms) {
        if (vm->startsWith(query->searchTerm()))
            query->addMatch(std::shared_ptr<Item>(vm->produceItem())); // Implicit move
    }
}
