// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
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

#include "sshitem.h"
#include <memory>
#include <QProcess>
#include "shlex.h"
#include "standardaction.h"

using std::shared_ptr;
using Core::StandardAction;

extern QString terminalCommand;

Ssh::Item::Item(const QString &id, const QString &connector) : Core::StandardItem(id), connector_(connector) {
    shared_ptr<StandardAction> sa = std::make_shared<StandardAction>();
    sa->setText(QString("Connect to '%1' using ssh").arg(connector));
    sa->setAction([this](){
        this->activate();
    });
    this->setActions({sa});
}

Ssh::Item::Item(const Ssh::Item &copy) : Core::StandardItem(copy.id_), connector_(copy.connector_) {
    text_ = copy.text_;
    subtext_ = copy.subtext_;
    completion_ = copy.completion_;
    iconPath_ = copy.iconPath_;
    shared_ptr<StandardAction> sa = std::make_shared<StandardAction>();
    sa->setText(QString("Connect to '%1' using ssh").arg(connector_));
    sa->setAction([this](){
        this->activate();
    });
    this->setActions({sa});
}

void Ssh::Item::activate() const {
    QStringList tokens;
    tokens << Util::ShellLexer::split(terminalCommand)
           << "ssh"
           << Util::ShellLexer::split(connector_);
    QProcess::startDetached(tokens.takeFirst(), tokens);
}

void Ssh::Item::setConnector(const QString &connector)
{
    connector_ = connector;
}
