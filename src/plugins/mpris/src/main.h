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

#pragma once
#include <QObject>
#include <QPointer>
#include "player.h"
#include "command.h"
#include "extension.h"
#include "queryhandler.h"
#include <QDBusConnection>
using Core::Query;

class QDBusMessage;

namespace MPRIS {

class ConfigWidget;

class Extension final :
        public QObject,
        public Core::Extension,
        public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:
    Extension();
    ~Extension();

    /*
     * Implementation of extension interface
     */

    QWidget *widget(QWidget *parent = nullptr) override;
    void setupSession() override;
    void handleQuery(Query *query) override;
    QString name() const override { return name_; }


    /*
     * Extension specific members
     */

private:
    //static QRegExp filterRegex;
    const char* name_ = "MPRIS Control Center";
    static QDBusMessage findPlayerMsg;
    QPointer<ConfigWidget> widget_;
    //QStringList mediaPlayers;
    QList<Player*> mediaPlayers;
    QStringList commands;
    QMap<QString, Command> commandObjects;
};
}
