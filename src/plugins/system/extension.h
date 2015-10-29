// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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
#include <QPointer>
#include <QIcon>
#include <vector>
#include "iextension.h"
using std::vector;

class StandardItem;

namespace System {

class ConfigWidget;

class Extension final : public IExtension
{
    Q_OBJECT
    Q_INTERFACES(IExtension)
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")


public:
    Extension();
    ~Extension();

    enum class Actions { POWEROFF, REBOOT, SUSPEND, HIBERNATE, LOGOUT, LOCK, NUM_ACTIONS };

    // IExtension
    QWidget *widget(QWidget *parent = nullptr) override;
    vector<shared_ptr<AlbertItem>> staticItems() const override;

    // API special to this extension
    QString command(Actions action);
    void setCommand(Actions action, const QString& cmd);
    void restoreCommands();

private:
    QPointer<ConfigWidget> widget_;
    vector<shared_ptr<AlbertItem>> index_;

    vector<QString> titles_;
    vector<QString> descriptions_;
    vector<QString> iconpaths_;
    vector<QString> defaults_;
};
}
