// albert - a simple application launcher for linux
// Copyright (C) 2014-2016 Manuel Schneider
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
#include <QFileSystemWatcher>
#include <set>
#include "abstractextension.h"

namespace Terminal {

class ConfigWidget;

class Extension final : public AbstractExtension
{
    Q_OBJECT
    Q_INTERFACES(AbstractExtension)
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();

    /*
     * Implementation of extension interface
     */

    QString name() const override { return "Terminal"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void teardownSession() override;
    void handleQuery(AbstractQuery * query) override;
    bool runExclusive() const override {return true;}
    QStringList triggers() const override {return {">"};}

private:

    void rebuildIndex();

    QPointer<ConfigWidget> widget_;
    QFileSystemWatcher watcher_;
    std::set<QString> index_;
    bool dirtyFlag_;
    QString iconPath_;

};
}
