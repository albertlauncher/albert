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
#include <QAbstractTableModel>
#include <QFileSystemWatcher>
#include <QPointer>

#include <memory>
#include <vector>
using std::shared_ptr;
using std::vector;

#include "iextension.h"

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

    /*
     * Implementation of extension interface
     */

    QString name() const override { return "Websearch"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Query query) override;
    bool runExclusive() const {return true;}
    QStringList triggers() const override;
    ItemSPtrVec fallbacks() const override;

    /*
     * Extension specific members
     */

public slots:

    void restoreDefaults();

private:
    QPointer<ConfigWidget> widget_;
    std::vector<shared_ptr<SearchEngine>> index_;
};

}
