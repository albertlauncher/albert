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
#include <QString>
#include <QStringList>
#include <vector>
#include <memory>
using std::vector;
using std::unique_ptr;
#include "pluginloader.h"


class PluginManager final : public QObject {
    Q_OBJECT

public:
    PluginManager();
    ~PluginManager();

    void refreshPluginSpecs();
    const vector<unique_ptr<PluginSpec>> &plugins();
    void loadPlugin(const unique_ptr<PluginSpec> &plugin);
    void unloadPlugin(const unique_ptr<PluginSpec> &plugin);
    void enablePlugin(const unique_ptr<PluginSpec> &plugin);
    void disablePlugin(const unique_ptr<PluginSpec> &plugin);
    bool pluginIsEnabled(const unique_ptr<PluginSpec> &plugin);
    void storeConfiguration();

private:
    vector<unique_ptr<PluginSpec>> plugins_;
    QStringList blacklist_;
    static const QString CFG_BLACKLIST;

signals:
    void pluginsChanged();
    void pluginLoaded(QObject*);
    void pluginAboutToBeUnloaded(QObject*); // MUST BLOCK
};
