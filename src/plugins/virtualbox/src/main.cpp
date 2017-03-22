// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
// Contributed to by 2016 Martin Buergmann
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
#include "main.h"
#include "vm.h"
#include "query.h"
#include "xdgiconlookup.h"
#include "standarditem.h"
#include "standardaction.h"
using Core::Action;
using Core::StandardAction;
using Core::StandardItem;



class VirtualBox::VirtualBoxPrivate
{
public:
    VirtualBoxPrivate(Extension *q) : q(q) { }
    Extension *q;

    QPointer<ConfigWidget> widget;
    QList<VM*> vms;
    QFileSystemWatcher vboxWatcher;

    void rescanVBoxConfig(QString path);

};



/** ***************************************************************************/
void VirtualBox::VirtualBoxPrivate::rescanVBoxConfig(QString path) {

    qDebug() << "Start indexing VirtualBox images.";

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

    qDebug() << qPrintable(QString("Indexed %2 VirtualBox images.").arg(found));
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
VirtualBox::Extension::Extension()
    : Core::Extension("org.albert.extension.virtualbox"),
      Core::QueryHandler(Core::Extension::id),
      d(new VirtualBoxPrivate(this)) {

    VMItem::iconPath_ = XdgIconLookup::iconPath("virtualbox");
    if ( VMItem::iconPath_.isNull() )
        VMItem::iconPath_ = ":vbox";

    QString vboxConfigPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, "VirtualBox/VirtualBox.xml");
    if (vboxConfigPath.isEmpty())
        throw "VirtualBox was not detected!";

    d->rescanVBoxConfig(vboxConfigPath);
    d->vboxWatcher.addPath(vboxConfigPath);
    connect(&d->vboxWatcher, &QFileSystemWatcher::fileChanged,
            std::bind(&VirtualBoxPrivate::rescanVBoxConfig, d.get(), std::placeholders::_1));
}



/** ***************************************************************************/
VirtualBox::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *VirtualBox::Extension::widget(QWidget *parent) {
    return new QWidget(parent);
}



/** ***************************************************************************/
void VirtualBox::Extension::setupSession() {
    for (VM *vm : d->vms)
        vm->probeState();
}



/** ***************************************************************************/
void VirtualBox::Extension::handleQuery(Core::Query * query) {
    for (VM* vm : d->vms) {
        if (vm->startsWith(query->searchTerm()))
            query->addMatch(std::shared_ptr<Item>(vm->produceItem()));
    }
}
