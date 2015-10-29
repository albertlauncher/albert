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
#include <QAbstractTableModel>
#include <QFileSystemWatcher>
#include <QPointer>
#include <QIcon>
#include <memory>
#include "iextension.h"
using std::vector;
using std::shared_ptr;

namespace Websearch {

class SearchEngine;
class ConfigWidget;



class Extension final : public IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")
    Q_INTERFACES(IExtension)


public:
    Extension();
    ~Extension();

    // GenericPluginInterface
    QWidget *widget(QWidget *parent = nullptr) override;

    // IExtension
    void handleQuery(shared_ptr<Query> query) override;
    void handleFallbackQuery(shared_ptr<Query> query) override;
    bool isTriggerOnly() const override {return true;}
    bool runExclusive() const override {return true;}
    QStringList triggers() const override;

    // API special to this extension
    void restoreDefaults();


private:
    QPointer<ConfigWidget> widget_;
    vector<shared_ptr<SearchEngine>> index_;

};

}
